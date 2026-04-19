[English](README.md)
|
中文

# 小企鹅输入法 iOS 版

[Fcitx5](https://github.com/fcitx/fcitx5) 输入框架的 iOS 移植。

目前处于开发者测试阶段，请使用 [SideStore](https://github.com/SideStore/SideStore) 安装。

注意：没有开发者账号，App Group 无法使用，请您
* 授权键盘完全访问；
* 每次更改数据或配置项后，点击 `同步配置`。

## iOS 16.1 分支（ios-16.1-rebase）

本分支在上游 `ios-16.1-rebase` 基础上，把最低部署目标从 iOS 16.3 降到 **iOS 16.1**，
使 TrollStore 等工具可以在 iOS 16.1 真机安装使用。

**验证记录（2026-04-19）：已在 iPhone 13 (iPhone14,2) / iOS 16.1 (20B82) 真机上
成功安装并打开，不再出现启动闪退。**

### 相对上游的改动

| 改动 | 原因 |
|---|---|
| `CMakeLists.txt`：`DEPLOYMENT_TARGET` 16.3 → 16.1 | 支持 iOS 16.1 设备 |
| `CMakeLists.txt`：加 `_LIBCPP_DISABLE_AVAILABILITY` | 规避 Xcode 26 SDK 的 `<format>` 头文件把 `to_chars(float)` 标记为 iOS 16.3+ 导致的编译报错 |
| `CMakeLists.txt`：加 `-Wl,-weak-lc++` | 把 `to_chars(float)` 等 iOS 16.3+ 符号弱链接，真机上解析为 NULL；由于这些符号实际不会被调用，安全 |
| `CMakeLists.txt`：`option(MOZC "" OFF)` | fcitx5-prebuilder 的 libmozc 2026-04-16 重新上传后有 protobuf 版本不匹配，先禁用（不影响中文输入） |
| **新增 `common/libcxx_ios161_shim.cpp`** | **关键修复**：Xcode 26 的 libc++ 会调用 `std::__1::__hash_memory`（iOS 17+ 才有的符号）。iOS 16.1 上该符号不存在，弱链接解析为 NULL，任何 `unordered_map<string,...>` 插入都会崩在静态初始化阶段（fcitx addon 注册时必定触发）。shim 用 Murmur2 哈希实现该函数，直接编进每个可执行文件 |
| `src/CMakeLists.txt` / `keyboard/CMakeLists.txt` | 把 shim 直接加到主 app 和每个键盘扩展的源码列表里，保证每个 binary 都有本地强定义 |
| `scripts/generate-icons.sh`：`--minimum-deployment-target` 16.3 → 16.1 | actool 与部署目标对齐 |
| `.github/workflows/ci.yml` | 加 `ios-16.1-rebase` 分支触发；加 Verify 步骤用 `nm` 检查所有 binary 里 `to_chars(float)` 是弱引用且 `__hash_memory` 是本地定义（防止这两个关键问题回归） |
| `.github/workflows/ci.yml`：Pack IPA 前加 `codesign --force --sign -` | **关键修复**：CI 用 `CODE_SIGNING_ALLOWED=NO` 出来的 bundle 完全没签名，TrollStore 安装时的 ad-hoc 假签没有 baseline 可以读 App Group entitlement，于是 iOS 16.1 认为键盘扩展没 entitlement，**第三方 App（微信、咸鱼等）的键盘选择列表里不显示**（系统 App 如短信、备忘录走另一条较宽松的路径所以能用）。打包前用 `keyboard.entitlements` / `app.entitlements` 先 ad-hoc 签进每个 binary 的签名里，TrollStore 再签时就能继承到 App Group |

### 如何构建

直接在本 fork 的 `ios-16.1-rebase` 分支上推任意改动，GitHub Actions 会在
macOS 26 + Xcode 26.4 上自动构建并产出 `Fcitx5.ipa`，可用 TrollStore 侧载。
