#include "ardugirl/constants.hpp"
#include "ardugirl/platform.hpp"
#include "render.hpp"
#include "storage.hpp"
#include "crash.hpp"

#include <SDL.h>

#include <array>
#include <cstdio>

namespace ardugirl::platform {

namespace {

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;
detail::TexturePixels texture_pixels{};
std::uint64_t start_counter = 0;
std::uint64_t counter_frequency = 1;
std::uint8_t current_buttons = 0;
bool running = true;
bool headless = false;
bool invert = false;
SDL_AudioDeviceID audio_device = 0;
std::array<std::uint32_t, 2> audio_phase{};
std::array<std::uint32_t, 2> audio_step{};
const std::uint8_t* wave_samples = nullptr;
std::uint16_t wave_sample_count = 0;
std::uint64_t wave_position = 0;
std::uint64_t wave_step = 0;
std::array<SynthVoice, 4> synth_voices{};
std::array<std::uint32_t, 4> synth_phases{};
std::uint32_t noise_state = 1;

constexpr std::uint8_t kLeft = 0x01;
constexpr std::uint8_t kRight = 0x02;
constexpr std::uint8_t kUp = 0x04;
constexpr std::uint8_t kDown = 0x08;
constexpr std::uint8_t kA = 0x10;
constexpr std::uint8_t kB = 0x20;

std::uint8_t map_key(SDL_Keycode key) noexcept {
    switch (key) {
    case SDLK_LEFT: case SDLK_a: return kLeft;
    case SDLK_RIGHT: case SDLK_d: return kRight;
    case SDLK_UP: case SDLK_w: return kUp;
    case SDLK_DOWN: case SDLK_s: return kDown;
    case SDLK_z: case SDLK_j: return kA;
    case SDLK_x: case SDLK_k: return kB;
    default: return 0;
    }
}

void release_resources() noexcept {
    if (audio_device != 0) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}


void audio_callback(void*, Uint8* stream, int length) noexcept {
    auto* samples = reinterpret_cast<std::int16_t*>(stream);
    const auto count = length / static_cast<int>(sizeof(std::int16_t));
    for (int index = 0; index < count; ++index) {
        std::int32_t mixed = 0;
        for (std::size_t channel = 0; channel < audio_step.size(); ++channel) {
            if (audio_step[channel] != 0) {
                mixed += audio_phase[channel] < 0x80000000u ? 4000 : -4000;
                audio_phase[channel] += audio_step[channel];
            }
        }
        if (wave_samples != nullptr && wave_sample_count != 0) {
            const auto sample_index = static_cast<std::uint32_t>(wave_position >> 32u);
            if (sample_index < wave_sample_count) {
                mixed += (static_cast<std::int32_t>(wave_samples[sample_index]) - 128) * 48;
                wave_position += wave_step;
            } else {
                wave_samples = nullptr;
                wave_sample_count = 0;
            }
        }
        for (std::size_t channel = 0; channel < synth_voices.size(); ++channel) {
            const auto& voice = synth_voices[channel];
            if (voice.frequency_hz == 0 || voice.volume == 0) continue;
            synth_phases[channel] += static_cast<std::uint32_t>(
                (static_cast<std::uint64_t>(voice.frequency_hz) << 32u) / 48000u);
            std::int32_t signal = 0;
            if (voice.waveform == 2) {
                const auto ramp = static_cast<std::int32_t>(synth_phases[channel] >> 23u) & 0x1FF;
                signal = ramp < 256 ? ramp - 128 : 383 - ramp;
            } else if (voice.waveform == 3) {
                noise_state = (noise_state >> 1u) ^
                    (static_cast<std::uint32_t>(-(noise_state & 1u)) & 0xB400u);
                signal = (noise_state & 1u) != 0 ? 128 : -128;
            } else {
                signal = synth_phases[channel] < 0x80000000u ? 128 : -128;
            }
            mixed += signal * static_cast<std::int32_t>(voice.volume) / 2;
        }
        if (mixed > 32767) mixed = 32767;
        if (mixed < -32768) mixed = -32768;
        samples[index] = static_cast<std::int16_t>(mixed);
    }
}

void init_optional_audio() noexcept {
    if (headless) return;

    // WSLg 通过 PulseAudio socket 提供声音，但部分 SDL 构建仍会先选择没有默认声卡的 ALSA。
    // 尊重用户显式选择；仅在未指定驱动且检测到 WSLg/PulseAudio 环境时优先使用 pulseaudio。
    const auto* requested_driver = SDL_getenv("SDL_AUDIODRIVER");
    const auto* pulse_server = SDL_getenv("PULSE_SERVER");
    const auto initialized = requested_driver == nullptr && pulse_server != nullptr
        ? SDL_AudioInit("pulseaudio")
        : SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (initialized != 0) return;

    SDL_AudioSpec desired{};
    desired.freq = 48000;
    desired.format = AUDIO_S16SYS;
    desired.channels = 1;
    desired.samples = 512;
    desired.callback = audio_callback;
    audio_device = SDL_OpenAudioDevice(nullptr, 0, &desired, nullptr, 0);
    if (audio_device != 0) {
        SDL_PauseAudioDevice(audio_device, 0);
    }
}

} // 匿名命名空间

bool init(const Config& config) noexcept {
    linux_crash::install();
    running = true;
    current_buttons = 0;
    headless = config.headless;
    invert = config.invert;
    audio_phase.fill(0);
    audio_step.fill(0);
    wave_samples = nullptr;
    wave_sample_count = 0;
    wave_position = 0;
    wave_step = 0;
    synth_voices = {};
    synth_phases = {};
    noise_state = 1;

    // 无窗口测试仍初始化 SDL 的计时与事件子系统，但不要求显示服务器。
    const auto subsystems = static_cast<Uint32>(SDL_INIT_TIMER | SDL_INIT_EVENTS |
                                                (headless ? 0 : SDL_INIT_VIDEO));
    if (SDL_Init(subsystems) != 0) {
        std::fprintf(stderr, "SDL 初始化失败：%s\n", SDL_GetError());
        return false;
    }
    if (!linux_storage::init(config.game_id, config.save_dir)) {
        SDL_Quit();
        return false;
    }

    counter_frequency = SDL_GetPerformanceFrequency();
    start_counter = SDL_GetPerformanceCounter();

    if (headless) {
        return true;
    }

    const auto scale = config.scale == 0 ? 1 : config.scale;
    const auto flags = static_cast<Uint32>(SDL_WINDOW_RESIZABLE |
        (config.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    window = SDL_CreateWindow(config.title, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              kScreenWidth * scale, kScreenHeight * scale, flags);
    if (window == nullptr) {
        std::fprintf(stderr, "SDL 窗口创建失败：%s\n", SDL_GetError());
        shutdown();
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
                                               SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        // 软件渲染可覆盖虚拟机、远程桌面和缺少 GPU 驱动的开发环境。
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == nullptr || SDL_RenderSetLogicalSize(renderer, kScreenWidth,
                                                        kScreenHeight) != 0) {
        std::fprintf(stderr, "SDL 渲染器创建失败：%s\n", SDL_GetError());
        shutdown();
        return false;
    }
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                kScreenWidth, kScreenHeight);
    if (texture == nullptr) {
        std::fprintf(stderr, "SDL 纹理创建失败：%s\n", SDL_GetError());
        shutdown();
        return false;
    }
    // 窗口和渲染器先完成初始化，音频探测失败不得阻塞或隐藏游戏窗口。
    init_optional_audio();
    return true;
}

void shutdown() noexcept {
    release_resources();
    linux_storage::shutdown();
    SDL_Quit();
}

bool pump_events() noexcept {
    SDL_Event event{};
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            } else {
                current_buttons |= map_key(event.key.keysym.sym);
            }
        } else if (event.type == SDL_KEYUP) {
            current_buttons &= static_cast<std::uint8_t>(~map_key(event.key.keysym.sym));
        }
    }
    return running;
}

std::uint8_t buttons() noexcept {
    return current_buttons;
}

std::uint32_t millis() noexcept {
    const auto elapsed = SDL_GetPerformanceCounter() - start_counter;
    return static_cast<std::uint32_t>((elapsed * 1000u) / counter_frequency);
}

std::uint32_t micros() noexcept {
    const auto elapsed = SDL_GetPerformanceCounter() - start_counter;
    return static_cast<std::uint32_t>((elapsed * 1000000u) / counter_frequency);
}

void sleep_ms(std::uint32_t duration) noexcept {
    SDL_Delay(duration);
}

void set_tone(std::uint16_t frequency_hz, std::uint8_t channel) noexcept {
    if (audio_device == 0 || channel >= audio_step.size()) return;
    SDL_LockAudioDevice(audio_device);
    audio_step[channel] = static_cast<std::uint32_t>(
        (static_cast<std::uint64_t>(frequency_hz) << 32u) / 48000u);
    SDL_UnlockAudioDevice(audio_device);
}

void stop_tone(std::uint8_t channel) noexcept {
    if (audio_device == 0 || channel >= audio_step.size()) return;
    SDL_LockAudioDevice(audio_device);
    audio_step[channel] = 0;
    SDL_UnlockAudioDevice(audio_device);
}

void play_wave(std::uint16_t sample_rate_hz, const std::uint8_t* samples,
               std::uint16_t sample_count) noexcept {
    if (audio_device == 0 || samples == nullptr || sample_rate_hz == 0 || sample_count == 0) return;
    SDL_LockAudioDevice(audio_device);
    wave_samples = samples;
    wave_sample_count = sample_count;
    wave_position = 0;
    wave_step = (static_cast<std::uint64_t>(sample_rate_hz) << 32u) / 48000u;
    SDL_UnlockAudioDevice(audio_device);
}

void stop_wave() noexcept {
    if (audio_device == 0) return;
    SDL_LockAudioDevice(audio_device);
    wave_samples = nullptr;
    wave_sample_count = 0;
    SDL_UnlockAudioDevice(audio_device);
}

void set_synth(const SynthVoice* voices, std::uint8_t count) noexcept {
    if (audio_device == 0) return;
    SDL_LockAudioDevice(audio_device);
    synth_voices = {};
    if (voices != nullptr) {
        const auto copied = count < synth_voices.size() ? count : synth_voices.size();
        for (std::size_t index = 0; index < copied; ++index) synth_voices[index] = voices[index];
    }
    SDL_UnlockAudioDevice(audio_device);
}

void present(const Framebuffer::Storage& pixels) noexcept {
    if (headless || texture == nullptr) {
        return;
    }
    detail::convert_framebuffer(pixels, invert, texture_pixels);
    SDL_UpdateTexture(texture, nullptr, texture_pixels.data(),
                      kScreenWidth * static_cast<int>(sizeof(std::uint32_t)));
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

bool storage_read(std::uint16_t offset, void* destination,
                  std::uint16_t size) noexcept {
    return linux_storage::read(offset, destination, size);
}

bool storage_write(std::uint16_t offset, const void* source,
                   std::uint16_t size) noexcept {
    return linux_storage::write(offset, source, size);
}

} // 命名空间 ardugirl::platform
