# FuckHypervConnectBar

![](assets/image.png)

The `Remote Desktop Connect` app provides us with an option to hide connect-bar forever, but Hyperv's `vmconnect.exe` does not!  This horrible ugly unclosable floating bar is annoying!

**Let's fuck it.**

## Usage

1. Start and enter into any Hyperv VM
2. Execute `FuckHypervConnectBar.exe`
3. Wait until `[INFO] Success.`
4. Maximum the Window of Hyperv VM and unpin the connection bar. The connection bar will hide and never show again.

## Principle

- The launcher inject `ApiHooker.dll` into `vmconnect.exe`.
- `ApiHooker.dll` hook winapi `ShowWindow`. When it find that `ShowWindow` is called to show this connect-bar，just return TRUE without calling real `ShowWindow`.
- connect-bar's window name: `BBar`, window class name: `BBarWindowClass`

## Tips

![](assets/tips.jpg)