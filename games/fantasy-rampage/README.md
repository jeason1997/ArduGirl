# Fantasy Rampage

元素卡牌策略游戏，复用公共 ATMlib 四声道播放器。

状态：`partial`。现有截图记录了启动、菜单和对局入口，但截图输入尚未固化为自动回放；完整对局与长期音乐仍待验收。

构建并运行：`make PLATFORM=linux GAME=fantasy-rampage`

仅构建：`make PLATFORM=linux GAME=fantasy-rampage build`

定向测试：`make PLATFORM=linux GAME=fantasy-rampage test-fantasy-rampage`

## ArduGirl SDL2 实际运行截图

启动阶段：![启动](assets/title.png)

菜单阶段：![菜单](assets/menu.png)

固定输入玩法画面：![玩法](assets/gameplay.png)
