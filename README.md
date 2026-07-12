# WBoard-Collab

> 基于开源项目 [WBoard](https://github.com/qicecrem/WBoard-Collab) 开发的交互式电子白板教学系统，新增实时多人协作功能。

WBoard 原为 Windows Qt5 平台的白板教学软件（OpenBoard/Open-Sankore 分支）。本项目在完成 Qt5→Qt6+CMake 跨平台迁移的基础上，独立设计实现了基于 WebSocket 的实时多人协作模块。

## 功能演示

```
教师机 (Host)                      学生机 (Client)
┌──────────────────┐              ┌──────────────────┐
│  白板 + 协作面板  │              │  白板 + 协作面板  │
│                  │   WebSocket  │                  │
│  信令服务器 :9876 │◄────────────►│  自动发现 (UDP)   │
│  UDP 广播        │              │                  │
└──────────────────┘              └──────────────────┘
```

- **实时笔迹同步**: Pen/Marker/Line 工具，支持压感，Bezier 插值
- **擦除同步**: Eraser 多边形布尔减法跨网络同步
- **Undo/Redo 同步**: 基于 QUndoStack 的远程撤销/重做
- **页面操作同步**: 添加/删除/切换页面
- **UDP 服务发现**: 局域网零配置，自动发现协作会话
- **防循环保护**: `mApplyingRemote` 守卫防止分布式回音

## 技术架构

```
┌─────────────────────────────────────────────────┐
│                  WBoard App                      │
│  ┌───────────┐   ┌────────────────────────────┐ │
│  │WBGraphics │   │ WBCollaborationManager     │ │
│  │  Scene    │──▶│ - 序列化/反序列化 stroke    │ │
│  └───────────┘   │ - Undo/Redo 远程同步       │ │
│                  │ - mApplyingRemote 守卫      │ │
│  ┌───────────┐   └─────────┬──────────────────┘ │
│  │WBBoard    │             │                     │
│  │Controller │   ┌─────────▼──────────────────┐ │
│  └───────────┘   │ WBCollaborationClient      │ │
│                  │ (QWebSocket + JSON)         │ │
│                  └─────────┬──────────────────┘ │
│                  ┌─────────▼──────────────────┐ │
│                  │ WBSignalingServer          │ │
│                  │ (内嵌信令服务器)             │ │
│                  └────────────────────────────┘ │
└─────────────────────────────────────────────────┘
```

### 数据流

```
用户画一笔
  → WBBoardView::mouseReleaseEvent
  → WBGraphicsScene::inputDeviceRelease
  → emit strokeCompleted(mCurrentStroke, tool)    [新增信号]
  → WBCollaborationManager::onStrokeCompleted
  → JSON 序列化 → WebSocket → 远程端
  → applyRemoteStroke → inputDevicePress/Move/Release
```

### 消息协议

| 类型 | 方向 | 说明 |
|------|------|------|
| `stroke` | 双向 | 笔迹数据：工具、颜色、线宽、压感点序列 |
| `erase` | 双向 | 擦除数据：被删 UUID、新增多边形 JSON |
| `command` | 双向 | Undo/Redo 指令 |
| `pageAdd/pageDelete/pageSwitch` | 双向 | 页面操作 |
| `join/leave/users` | Server→Client | 房间管理 |

## 构建

### 依赖

- Qt 6.2+ (Core, Gui, Widgets, WebSockets, Network, WebEngine, Multimedia, Svg)
- CMake 3.16+
- OpenSSL, ZLIB
- quazip (optional, for document compression)

### Linux

```bash
sudo apt install qt6-base-dev qt6-websockets-dev qt6-webengine-dev \
  qt6-multimedia-dev libquazip1-qt6-dev libssl-dev

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./WBoard
```

### Windows

```powershell
# 使用 MSVC 2022 + Qt 6.x
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=C:\Qt\6.x\msvc2022_64
cmake --build . --config Release
```

## 项目结构

```
WBoard-Collab/
├── WBoard/Source/
│   ├── collaboration/          # ★ 新增协作模块 (12 文件, ~1800 行)
│   │   ├── WBCollaborationMessage.h     # 协议常量
│   │   ├── WBSignalingServer.h/.cpp     # WebSocket 信令服务器
│   │   ├── WBCollaborationClient.h/.cpp # WebSocket 客户端
│   │   ├── WBCollaborationDiscovery.h/.cpp # UDP 广播发现
│   │   ├── WBCollaborationManager.h/.cpp # 核心编排层
│   │   └── WBCollaborationPalette.h/.cpp # UI 面板
│   ├── domain/                 # 场景图 (修改 WBGraphicsScene)
│   ├── board/                  # 画板控制 (修改 WBBoardController/PaletteManager)
│   ├── core/                   # 应用核心 (修改 WBApplication)
│   ├── gui/                    # UI 组件
│   ├── adaptors/               # 序列化/持久化 (SVG/CFF)
│   ├── web/                    # 浏览器 (WebEngine)
│   ├── tools/                  # 教具 (圆规/三角板/量角器)
│   └── ...
├── test_collaboration.cpp      # ★ 协作模块测试 (30 用例)
└── CMakeLists.txt              # CMake 构建
```

## 测试

```bash
# 编译并运行协作模块测试
g++ -std=c++17 -I WBoard/Source -I WBoard/Source/collaboration \
  $(pkg-config --cflags --libs Qt6Core Qt6WebSockets Qt6Network) \
  WBoard/Source/collaboration/*.cpp \
  build/WBoard_autogen/*/moc_WBSignalingServer.cpp \
  build/WBoard_autogen/*/moc_WBCollaborationClient.cpp \
  test_collaboration.cpp -fPIC -o test_collaboration

./test_collaboration
# RESULTS: 30 passed, 0 failed
```

测试覆盖：
- 信令服务器启动/端口绑定
- 客户端连接/断开
- 单房间多用户
- Stroke 消息往返
- Erase 消息往返
- 页面操作消息往返
- 用户离开/断线通知

## 版本历史

| 提交 | 内容 |
|------|------|
| `fa20a76` | 协作模块基础：Server/Client/Discovery/Manager/Palette |
| `78c40cb` | 集成到 WBApplication + WBBoardController |
| `87c7f17` | UI 面板挂载到右侧 Dock |
| `02c5b9f` | Eraser + Undo/Redo 同步 |
| `09a651a` | 页面增删切换同步 |
| `6a89a86` | 30 条自动化测试 |
| `9227c96` | Windows 跨平台兼容 |

## 致谢

- 原始项目 [WBoard](https://github.com/) — GPL 开源白板软件
- 上游 [OpenBoard](https://github.com/OpenBoard-org/OpenBoard) — 交互式白板教学平台
- [Open-Sankoré](https://en.wikipedia.org/wiki/Open-Sankor%C3%A9) — 原始项目起源

## 许可

基于原项目 GPL v3 许可。协作模块为原创新增代码，遵循相同许可。

---

*本项目为课程/实验室项目，在指导老师指导下独立完成。*
