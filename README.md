<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# 抽群U的小工具

**2020-9-20 归档**[^1]

**2026-11-1 <新> 重新整理、提交、归档**[^2]

> 某日，MRG 突然特高兴，于是打算在群里 roll 疯四v50，就诞生了这个东西，最后也确确实实是 Roll 了的，
> 某中奖群U：比起 kun 哥，我觉得mrg1337 (即群主 @MRG1337 [^3] / M.R.G.) 更牛逼

---

- 这个东西是随手瞎弄的，没有什么很高级的东西，不过依然是初学者可以参考的东西，一些东西我自以为是不赖的
- 现已归档
- <新> 上述“现已归档”发生在 2020-9-20，由于某些原因，旧的库没了，因此我整理了手头有的东西放到一起重发
- <新> 依然采用 GPL-3.0-or-later
- <新> 弄好之后依然是归档
- <2026-11-2> 我删除并彻底清理掉了潜在的可能侵权的资源文件，并为一切非代码文件添加 CC BY-SA 4.0 许可证，在 README 里添加了许可声明部分

---

## 许可声明

本仓库采用分许可模式：

- **代码** —— 所有源代码文件（包括但不限于 `.py`、 `.js`、 `.cpp`、 `.hpp` 等）采用 **GPL-3.0-or-later** 许可。完整文本见 [`LICENSE`](LICENSE) 文件。
- **文档与资源** —— `/randomly_pick/res` 目录、 `README.md`，以及仓库中其他一切非代码文件，采用 **CC BY-SA 4.0** 许可。完整文本见 [`LICENSE-CONTENT`](LICENSE-CONTENT) 文件。

这两部分相互独立。使用本仓库时，请遵守你所使用部分对应的许可条款。

兼容性说明：CC BY-SA 4.0 与 GPLv3 存在单向兼容关系。你可以改编本仓库中 CC BY-SA 4.0 的内容，并以 GPLv3 发布你的贡献，但反向不成立：本仓库中 GPL 许可的代码，不得改编后以 CC BY-SA 4.0 名义发布，也不得作为 CC BY-SA 4.0 内容的组成部分再分发。

---

## Win32 随机整数抽取器

> 前面说过，这个东西是随手瞎弄的，没有什么很高级的东西，因此是极为简单的，就都是最基本的东西，是一个最简单最简单的Win32
> 应用，一个**Win32随机整数抽取器**。由于我闲来无事（被隔离了），所以就在这里讲一讲这个简单的东西吧。

### 文件的头部：编码

```cpp
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
/*
* UNICODE / _UNICODE 两个宏必须在包含 <windows.h> 之前定义，否则会链接到 ...A(ANSI)版本的 API。这里手动定义，
* 保证程序用宽字符(W 后缀)API 编译。
*/
```

```cpp
#include <windows.h>  // 可理解为通往 WinAPI 的 “入口头文件”。
#include <string>  // 标准库头文件，提供 std::string 类，用于方便的表示和操作字符串
#include <random>  // <random> 用于 std::mt19937 和 std::uniform_int_distribution，是 C++ 标准库的随机数设施。
```

```cpp
#if defined(_MSC_VER)
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#endif
// 告诉 MSVC 链接器：这是 GUI 子系统程序（没有控制台窗口），入口点是 wWinMain。如果遗漏，程序会尝试找 main 而链接失败。
```

---

### 常量与全局变量

```cpp
#define IDI_APPICON 101
#define IDC_EDIT_MIN       1001
#define IDC_EDIT_MAX       1002
#define IDC_BTN_GENERATE   1003
#define IDC_STATIC_RESULT  1004
```

- 控件 ID，供 `WM_COMMAND` 中识别控件，也用于 `GetDlgItem` 一类操作。 `IDI_APPICON` 是资源里图标 ID。
- `g_hEditMin` 等三个全局 `HWND` 保存控件句柄，便于在 `OnGenerate` 中访问。
- `static std::mt19937 g_rng(std::random_device{}());` 全局 Mersenne Twister 引擎，用 `random_device` 做种子（真随机种子），只初始化一次。

> M! 注意：全局变量在 GUI 程序中常见，但严格来说"全局可变状态"不如把句柄放窗口 user data 优雅，不过这个小程序足够（懒得弄）。

---

### 辅助函数

- `GetWindowTextString`

```cpp
static std::wstring GetWindowTextString(HWND hWnd)
{
    int len = GetWindowTextLengthW(hWnd);
    std::wstring str(len + 1, L'\0');
    GetWindowTextW(hWnd, &str[0], len + 1);
    str.resize(len);
    return str;
}
```

- 先取长度，分配 len+1 个字符，多留 1 个给 \0，因为 GetWindowTextW 会写入结尾空字符。
- 调用后再 resize(len) 去掉那个空字符。
- 这是读取编辑框文本的标准宽字符封装。

---

- 严格的整数解析 ———— `ParseInt`

```cpp
static bool ParseInt(const std::wstring& text, int& value)
{
    size_t first = text.find_first_not_of(L" \t\r\n");
    if (first == npos) return false;
    size_t last = text.find_last_not_of(L" \t\r\n");
    std::wstring trimmed = text.substr(first, last - first + 1);

    try {
        size_t pos = 0;
        int v = std::stoi(trimmed, &pos, 10);
        if (pos != trimmed.size()) return false;   // 有多余字符
        value = v;
        return true;
    } catch (...) {
        return false;                              // 溢出/非法
    }
}
```

关键点：
1. 手工 trim 空白：`std::stoi` 本身能跳过前导空白，但不会检查尾部，所以这里先 `trim` 再判断
`pos == trimmed.size()`，确保整串都是数字。
2. `pos != trimmed.size()` → 比如 `"12abc"` 解析出 `12` 但 `pos=2`，会被拒绝。
3. `catch(...)` 捕获 `std::invalid_argument`（如空串、非数字）和 `std::out_of_range`
（如 `99999999999` 溢出），返回 `false`。
4. 只接受**十进制**（基数 10）；负数 `-5` 是允许的（`stoi` 支持前导 `-`），这符合"标号"也可为负的
场景（必要的，我的群群主被标成 0 号，几个管理是 -6 到 -1，其他群U则是从 1 开始数）。

---

- `SetFontProc` —— 回调给子控件设字体

```cpp
static BOOL CALLBACK SetFontProc(HWND hChild, LPARAM lParam)
{
    SendMessageW(hChild, WM_SETFONT, (WPARAM)lParam, TRUE);
    return TRUE;
}
```

- 配合 `EnumChildWindows(hWnd, SetFontProc, ...)`，把窗口的所有子控件统一设为默认 GUI 字体，
避免默认的丑陋点阵字体。
- 返回 `TRUE` 表示继续枚举下一个子窗口。

---

### 核心逻辑：`OnGenerate`

```cpp
static void OnGenerate(HWND hWnd)
{
    int nMin = 0, nMax = 0;

    if (!ParseInt(GetWindowTextString(g_hEditMin), nMin))
    {
        MessageBoxW(..., L"请输入合法的最小值（整数）。", ...);
        SetFocus(g_hEditMin);
        return;
    }
    if (!ParseInt(GetWindowTextString(g_hEditMax), nMax)) { ... SetFocus(g_hEditMax); return; }

    if (nMin > nMax)
    {
        MessageBoxW(..., L"最大值必须大于或等于最小值。", ...);
        SetFocus(g_hEditMax);
        return;
    }

    std::uniform_int_distribution<int> dist(nMin, nMax);
    int result = dist(g_rng);

    wchar_t buffer[64];
    wsprintfW(buffer, L"%d", result);
    SetWindowTextW(g_hStaticResult, buffer);
}
```

流程：
1. 读取并解析两个编辑框，失败则弹警告并把**焦点移回错误框**（细节要做好）。
2. 检查区间合法性。
3. 构造 `uniform_int_distribution<int>(nMin, nMax)`，在闭区间 `[nMin, nMax]` 上均匀取整，无模偏差。
4. `wsprintfW` 把结果转成字符串，写进静态文本控件。

> 关于随机性质量： `std::mt19937` 周期很长、分布均匀，配合 `random_device` 播种，
> 对"抽个群U"这种用途绰绰有余。 `uniform_int_distribution` 内部会正确处理取模偏差，
> 不会出现"某些数字概率略高"。

---

### 窗口过程 `WncProc`

```cpp
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE: ...
    case WM_COMMAND: ...
    case WM_CLOSE:  DestroyWindow(hWnd); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    default: break;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
// WM_CREATE：一次性创建所有子控件
```

```cpp
HINSTANCE hInst = reinterpret_cast<LPCREATESTRUCTW>(lParam)->hInstance;
// lParam 是 CREATESTRUCT 指针，从中取 hInstance 给子控件用。
```

创建 5 个控件：
| 类型 | 作用 | 位置/大小 |
| --- | --- | --- |
| STATIC | “最小标号” | (15,28,70,22)，右对齐 `SS_RIGHT` |
| EDIT | 最小值输入，默认为 "1" | (95,25,130,26)， `WS_EX_CLIENTEDGE` 凹陷边框 |
| STATIC | “最大标号：” | (15,68,...) |
| EDIT | 最大值输入，默认 "100" | (95,65,...) |
| BUTTON | "开抽"， `BS_DEFPUSHBUTTON` 默认按钮 | (15,110,210,34) |
| STATIC | 结果显示， `SS_CENTER | SS_CENTERIMAGE` 居中 | (15,160,210,44)

细节：

- 编辑框 `WS_TABSTOP` 支持 `Tab` 切换焦点。
- 控件 ID 通过 `(HMENU)` 强转传入（子控件用 ID 而非菜单句柄）。
- 最后 `EnumChildWindows + SetFontProc` 给所有子控件设字体。

---

- `WM_COMMAND`：按钮点击

```cpp
if (LOWORD(wParam) == IDC_BTN_GENERATE && HIWORD(wParam) == BN_CLICKED)
    OnGenerate(hWnd);
// LOWORD(wParam) = 控件 ID，HIWORD(wParam) = 通知码。必须同时判断，否则编辑框其他通知也会误触发。
```

---

- `WM_CLOSE` / `WM_DESTROY`

1. `WM_CLOSE`：用户点 ×， `DestroyWindow` → 触发 `WM_DESTROY`。
2. `WM_DESTROY`： `PostQuitMessage(0)` 让消息循环退出。如果只处理 `WM_CLOSE`
而不发 `quit`，进程会残留后台。

其余消息交 `DefWindowProcW` 默认处理。

---

### 入口 `wWinMain`

```cpp
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    const wchar_t* kClassName = L"RandomIntWindowClass";

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;   // 尺寸变化时重绘
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APPICON));
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); // 系统灰色对话框背景
    wc.lpszClassName = kClassName;
    wc.hIconSm       = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APPICON));

    if (!RegisterClassExW(&wc)) { MessageBoxW(...); return 0; }
```

- `COLOR_BTNFACE + 1` 是 Win32 的惯用写法：取系统调色板索引对应的画刷，得到标准对话框灰底。
- 大/小图标都从资源 `IDI_APPICON` 加载（需 .rc 里定义图标资源，否则图标为空）。
- 注册失败弹错误框并返回。

---

```cpp
    HWND hWnd = CreateWindowExW(0, kClassName, L"抽个群U",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 340, 270,
        nullptr, nullptr, hInstance, nullptr);
```

1. 窗口样式：有标题栏、关闭按钮、最小化按钮，但没有 WS_THICKFRAME（不可拉伸）、没有最大化。对固定布局的小工
具很合适。
2. 尺寸 340×270 是整个窗口尺寸（含边框），客户区略小，因此控件坐标都留了余量，我这里直接是硬写明了的，
真正工作要注意。
3. 标题"抽个群U"，随你改。

---

```cpp
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        if (!IsDialogMessageW(hWnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return (int)msg.wParam;
}
```

消息循环关键点：

1. `GetMessageW(...) > 0`：同时处理返回 -1（出错）和 0（WM_QUIT）的情况，比常见的
`while(GetMessage(...))` 更严谨一些。
2. `IsDialogMessageW(hWnd, &msg)`：给非对话框窗口提供对话框式键盘导航——Tab 切换焦点、
Enter 触发默认按钮（ `BS_DEFPUSHBUTTON` ）、Esc 等。这是让普通窗口也有"对话框手感"的关键，
很多人会漏。
3. 返回值用 `msg.wParam`（ `WM_QUIT` 的退出码）。

---

## 总结一下

> 关于这个东西，两百行 C++ （去掉 Allman 风格更少了）是极为简单的东西，一般的没事儿干的闲人稍微想想就能做出的，
> 可以说是没有任何技术含量的，不过麻雀虽小，五脏俱全，正如我前面所说，这个东西是初学者可以参考的东西，我觉得还是蛮不错的

### 分点总结一下比较好的特点吧：

#### 1. 结构完整、麻雀虽小五脏俱全

一个最小可用的 GUI 程序该有的骨架全在：

- `wWinMain` 入口 → 注册窗口类 → 创建窗口 → 消息循环
- `WndProc` 消息分发（ `WM_CREATE` / `WM_COMMAND` / `WM_CLOSE` / `WM_DESTROY`）
- 控件创建、文本读写、字体设置、输入校验、随机数
- 两百行多点的代码，一屏能读完一块，不像大项目那样一上来就被一堆东西淹没。

#### 2. （我自以为的）细节是要比很多教程扎实的

包含了不少的网上的 Win32 教程会漏的东西，或者说也可能只是“我感觉”吧：

- `IsDialogMessageW` —— 让普通窗口也有 Tab / Enter 导航，90% 的入门教程会漏（肯定的，我当时学的时候就没有）。
- `GetMessageW(...)` > 0 —— 正确处理出错返回，而教程里常写成 `while (GetMessage(...))`。
- `ParseInt` 的 `trim + pos != size()` 尾部校验 + 异常捕获，是**教科书级的严格解析(doge)！**
- 用 `std::mt19937` + `uniform_int_distribution` 而不是 `rand() % n`，避开了模偏差。

#### 3. 注释少但命名清晰

其实是我压根懒得写注释，随手做的玩具为什么要写注释呢？不过嘛，我的习惯是比较优秀的(doge)，就是说啊，我这个函数名
定的很不错的，我惯于起一些一看就懂的函数名，并且我很讨厌函数名直接用拼音，像 `GetWindowTextString` / `ParseInt` /
`OnGenerate` 这种名字，超简单的，一看就懂，对吧，所以说是适合作为"读代码"的练习。

#### 4. 这个东西毕竟是出于实际场景而诞生的，是一个**真的能跑，真的有用**的小工具

不是 HelloWorld 、 HelloQt 这种玩具，你像这样的东西，你学完就能改成自己的小玩意，**学习的正反馈是不赖的**。

---

### 需要注意的局限性

#### 1. 老东西太老了，它代表的是“传统 Win32 写法”，不是现代 C++

- 全局 `HWND：g_hEditMin` 等三处全局状态，现代做法是放窗口 `GWLP_USERDATA` 或封装成类。
- 绝对坐标布局： `(15, 28, 70, 22)` 硬编码，学不到布局管理（Win32 本身也确实没布局器，但可以用 `WM_SIZE` 手动排），毕竟是随便弄的东西，所以硬编码这个坏习惯在我这里体现的比较明显。
- 没有 RAII： `HFONT`、图标等资源靠系统默认清理，没体现"资源获取即初始化"。
- `wsprintfW`：无长度参数，现代代码应 `swprintf_s` / `std::to_wstring`。

如果你学 Win32 是为了读懂/维护旧代码，或者只是玩，那没问题的，如果你想学现代 C++ 工程规范，这份代码饱含着我的坏习惯。

#### 2. 覆盖面窄

少很多东西的，就是最基础的东西，什么静态文本、编辑框、按钮、消息框啊之类的，它**没涉及到**：
菜单、工具栏、状态栏

- 列表/表格/树控件（ `ListView` / `TreeView`）
- 文件对话框（ `GetOpenFileName`）、注册表、多线程
- GDI 绘图、自定义控件、DPI 适配（这份代码在 4K 屏上可能模糊，虽然说我也买不起 3090）

简言之，学完这个，你只是相当于上了 Win32 的入门的第一堂课。

#### 3. Win32 本身学习曲线是陡峭的，这东西就是用 C++ 实现的风味 C (doge)

Win32 API 是 C 风格、消息驱动、参数多，像 `WPARAM `/ `LPARAM` 强转是满天飞的，对新手**显然不友好**。
**如果你只是想做个抽奖器，用 Python + tkinter 说不定十行就搞定**，学 Win32 的成本收益是要自己权衡的，
而且学 Win32 基本没啥用，上限就在这儿，意义不大，我前面提到过了，我是闲得没事儿干弄的。正经搞 GUI 建议转头去 Qt。

### 所以，我建议...

#### 这么学：
1. 第一遍：通读，然后让代码跑起来，理解理解"消息循环 -> 窗口过程 -> 控件"这条主线。
2. 第二遍：做改造练习，这个是最具价值的，Most Valuable Practice，MVP(doge)，比如你可以把三个全局 `HWND`
收进一个 `struct`，用 `SetWindowLongPtr(hWnd, GWLP_USERDATA, ...)` 挂到窗口上，**学窗口用户数据**。再比
如你可以吧 `wsprintfW` 换成 `std::to_wstring`，**学 C++ 字符串**。还有呢，给编辑框加 `ES_NUMBER`，或反过来支持小数，
学**控件样式**。抑或是加一个抽多少多少个不重复的按钮，学 `std::sample`。或者是加 `WM_GETMINMAXINFO` 限制窗口尺寸，
加 `WM_SIZE` 让控件随窗口缩放，可以学布局。我觉得吧，诸如此类，都是可以让一个 GUI 初学者受益匪浅的。
3. 第三遍：对照现代的方案，同样功能 + 一个小封装类怎么写呢？或者，用 WinUI / Qt / Dear ImGui 各写一遍，体会
原生 API vs 框架的差异。这样，你就能够理解 Win32 的抽象层级之低了，这个对学 GUI 绝对是有所受用的。
4. 延伸(扯远了)：你可以再进一步的探索，比如加多线程（后台算随机数，主线程更新 UI）那就学 `CreateThread` + 
`PostMessage`；加注册表保存上次输入就学 `RegSetValueEx`；加 DPI 适配->学 SetProcessDpiAwareness。

### 总结一下（这次是真的要结束了）

> 既然你都看到这儿了，想必你是知道我有到底是多无聊了，那把下面这段话看完也不迟

学习需要带批判的眼光，我这个东西既然说了是随时写的，那你就必须**明确一点**，就是这个东西是**不代表现代 C++ 工具规范**的，
你可以把它当成一个 "Win32 骨架图"，或者说 "扎实细节的正面例子"，这个是可获得一些“受用”的。除了这点之外，还有就是，如果
你的目标是快速做个小工具，那 Python （我并不喜欢这个）无疑是更简单的，除去 Python 也是有很多更省力的技术栈；但是如果你
的目标是理解 Windows 桌面程序怎么跑，这份代码是不错的一个起点的。

- MRG1337[^3]，写于2020-9-19[^4]，隔离ing

---

## <新的>

- <2026-10-31>，由于原仓库没了，我实际上只找到了 README.md 和那个 .cpp，完整的提交记录、 issue（其实就是和朋友的打屁）都没有了
- （不是所有仓库，但这个是的）只好是把这个 .cpp 和放进 MSVC,并且配了一个手头有的图标文件了，工具链是啥我也记不得了，马马虎虎弄吧
- <2026-11-01>，本 README.md 于此日完成修订

> [^1]: <新>这个东西原本是早在 2020 年 9 月下旬，20 号就归档了的，这次重新组装好了目录和文件，很多东西手里没有，记忆也迷糊，有可能新加的东西不一定适合这个库，比如图标。
> [^2]: <新>其实手头只摸到了二进制、一个.cpp（这个完全没动）和一份 README.md，代码完全没动，都是新东西配合它，README小改动（如您所见）。文档里凡是句首带<新>字样的，都是本次恢复新加的东西
> [^3]: <新>我曾经用过多个 ID，MRG1337 是其中一个，那会主要就用的这个，但这些 ID 都脱不开 "M.R.G." 这仨字母的。
> [^4]: <新>我生日在前一天，当时在因为 COVID-19 疫情在隔离，无所事事，闲到极致。
