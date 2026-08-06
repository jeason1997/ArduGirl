# PY32F0xx 固定 vendor 快照

本目录保存 ArduGirl 的 PY32F002A 构建所需底层文件，导入来源为：

```text
E:/Projects/MCU_Project/32/Py32/Common
```

导入日期为 2026-08-06。快照包含 CMSIS Cortex-M0+ 核心头、PY32F002Ax5 设备头、所需 LL 头和源文件、系统初始化、启动汇编、32 KB Flash/4 KB RAM 链接脚本、OpenOCD target 配置及固定版本 OpenOCD 安装脚本。

上游文件的版权和许可声明保留在各文件头部。`py32f002a_32k_4k.ld`、设备容量定义、OpenOCD 配置及安装脚本来自参考工程针对实测 32 KB Flash、4 KB RAM 所做的固定配置。更新快照时必须重新核对这些修改并完成冷构建、烧录和调试验证。

`tools/` 是安装脚本生成的本地缓存，不进入版本控制。
