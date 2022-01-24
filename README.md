# OpenGL 材質讀取外部圖片教學

## 圖片讀取函式庫
如果想要特別讀取 BMP、JPEG 或 PNG 檔案，通常都會需要撰寫 Reder，但除了 BMP 之外其他的格式讀取或寫入都較為複雜，所以說一般建議使用已經開發好的函式庫即可。

### stb_image
#### 安裝方法 1: 直接下載 （推薦方法）
1. 首先下載 `stb_image.h` 到專案根目錄，[Github 載點](https://github.com/nothings/stb/blob/master/stb_image.h)。
2. 之後創建一個新的 `.cpp` 檔案，名稱隨意，裡面內容如下:
```c++=
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
```
3. 在要使用的檔案中（例如`main.cpp`）引入標頭檔，即可開始使用 stb 圖片函式庫了!
```c++=
#include "stb_image.h"
```

#### 安裝方法 2: 透過 vcpkg
1. 首先輸入指令來安裝 `stb` 函式庫。
```bash=
$ vcpkg install stb --triplet=x64-windows
```
2. 安裝好後，只需要在專案根目錄下的 `CMakeLists.txt` 加上相關設定
```cmake=
find_path(STB_INCLUDE_DIRS "stb.h")
target_include_directories(目標名稱 PRIVATE ${STB_INCLUDE_DIRS})
```
3. 之後創建一個新的 `.cpp` 檔案，名稱隨意，裡面內容如下:
```c++=
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
```
3. 在要使用的檔案中（例如`main.cpp`）引入標頭檔，即可開始使用 stb 圖片函式庫了!
```c++=
#include <stb_image.h>
```

#### 使用方法
讀取圖片的方式，就是這麼如此簡單：
```c++
int width, height, nrChannels;
unsigned char *image = stbi_load("圖片位置.png", &width, &height, &nrChannels, 0);

glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
```

由於 OpenGL 的 Texture Coordinate 中，原點 `(0, 0)` 是在左下角，而一般來說圖片索引的 `(0, 0)` 會是左上角（想想陣列的概念），所以讀取出來的圖片會是上下顛倒的，這個時候可以透過下列函式來矯正它：
```c++=
stbi_set_flip_vertically_on_load(true);
```
> 記得要在呼叫 `stbi_load()` 之前。

### SDL Image
除了使用 `stb_image.h` 之外，還可以使用 SDL2 自己的衍生函式庫 `SDL2-Image` 來實現讀取圖片的功能。
#### 安裝方法
1. 首先輸入指令來安裝 `sdl2-image` 函式庫。
```bash=
$ vcpkg install sdl2-image[core,libjpeg-turbo] --triplet=x64-windows
```
2. 安裝好後，記得要去專案根目錄下的 `CMakeLists.txt` 做相關設定:
```cmake=
find_package(sdl2-image CONFIG REQUIRED)
target_link_libraries(目標名稱 PRIVATE SDL2::SDL2_image)
```
3. 接著引入標頭檔並且初始化，這樣就可以開始使用 `SDL2-Image` 了。
```c++=
#include <SDL_image.h>

int main(int argc, char **argv) {
    // ... SDL 初始化
    
    // ... SDL Image 初始化
    int sdl_img_init_flags = IMG_INIT_JPG | IMG_INIT_PNG;
    int img_init = IMG_Init(sdl_img_init_flags);
    if ((img_init & sdl_img_init_flags) != sdl_img_init_flags) {
        std::cout << "Oops! Failed to initialize SDL Image extension library. :(\n" << SDL_GetError() << std::endl;
        return -1;
    }
    
    // 其他程式 ....
}

```
#### 使用方法
讀取圖片的方式，就是這麼如此簡單：
```c++=
SDL_Surface* image = IMG_Load("圖片位置.png");

// 抓取圖片寬、高與通道
int width = image->w;
int height = image->h;
int nrChannels = image->format->BitsPerPixel / 8;

glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
```

很可惜的是，`SDL2-Image` 沒有提供垂直翻轉的函式可以呼叫，所以這邊只能自己寫函式去翻轉圖片，或者是另一個方法..，把 Texture Coordinate 的 Y 顛倒過來（雖然是最簡單但是不建議）。
```c++
void flip_surface(SDL_Surface* surface) {
    SDL_LockSurface(surface);

    int pitch = surface->pitch; // row size
    char* temp = new char[pitch]; // intermediate buffer
    char* pixels = (char*) surface->pixels;

    for(int i = 0; i < surface->h / 2; ++i) {
        // get pointers to the two rows to swap
        char* row1 = pixels + i * pitch;
        char* row2 = pixels + (surface->h - i - 1) * pitch;

        // swap rows
        memcpy(temp, row1, pitch);
        memcpy(row1, row2, pitch);
        memcpy(row2, temp, pitch);
    }

    delete[] temp;

    SDL_UnlockSurface(surface);
}

// 上下翻轉圖片
SDL_Surface* image = IMG_Load("assets/textures/rickroll.png");
flip_surface(image);
```

## 利用 CMake 建立 Symbolic Link
當你有開始有讀取外部圖片的需求時，就會發現圖片路徑是一個很大的問題，比如說你在主程式寫說要讀取 `assets\textures\doge.png`（相對路徑寫法），但編譯後執行程式卻說一直找不到該檔案，除非你改成絕對路徑寫法，比如 `C:\uers\user\Desktop\CGHomework05\assets\textures\doge.png` 才可以讀取，但問題是這樣的寫法很不明智，因為代表就已經寫死你的專案必須要放在桌面，一旦改變了路徑或是換了地方又整個要重改。
之所以會有這樣的原因，就是因為你認為的【相對】路徑中的相對是基於 `main.cpp`，而事實上不是，是【相對於你最終編譯出來的執行檔】，舉例來說使用 Visual Studio 編譯的話，最終的程式（`.exe`）會產生在路徑 `CGHomework05\out\build\x64-Debug` 當中，所以說在 `main.cpp` 所寫的圖片路徑（假設是 `assets\textures\doge.png`），程式的路徑反而是會抓取 `CGHomework05\out\build\x64-Debug\assets\textures\doge.png`，而非我們預期的 `CGHomework05\assets\textures\doge.png`。
或許會有人就乾脆把在專案根目錄（`CGHomework05`）下的外部資源檔案全部手動複製到建置目錄下（`CGHomework05\out\build\x64-Debug\`），雖然可以解決方法但一樣換湯不換藥，畢竟建置目錄內的東西可是會隨著不同電腦、不同環境、不同 IDE、不同建置器和不同編譯器就會有所不一樣，所以說強烈不建議這樣做。
最好的解法就是透過 CMake 來自動幫我們處理，只要輸入以下語法即可：
```cmake=
add_custom_command(TARGET 目標名稱 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:目標名稱>/assets"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" # Make sure the directory exists
    COMMENT "Creating symlink from build tree to project resources..."
    VERBATIM
)
```
### 指令解釋
```cmake
add_custom_command(TARGET <target>
                   PRE_BUILD | PRE_LINK | POST_BUILD
                   COMMAND command1 [ARGS] [args1...]
                   [COMMAND command2 [ARGS] [args2...] ...]
                   [BYPRODUCTS [files...]]
                   [WORKING_DIRECTORY dir]
                   [COMMENT comment]
                   [VERBATIM] [USES_TERMINAL]
                   [COMMAND_EXPAND_LISTS])
```
* `add_custom_command()` 代表是我們可以透過該函式來執行一些自定義的指令，來達成一些我們可能會需要的事情，大概就式輸入指令到終端機的概念。
* `POST_BUILD` 意思是【建置後事件】，也就說當程式建置好後才會觸發指令，除此之外還有 `PRE_BUILD` 和 `PRE_LINK`。
* `COMMAND` 就是用來執行命令的意思。
* `DEPENDS` 就是依賴條件，當某檔案不存在時將會出錯。
* `VERBATIM` 大概就是命令的所有參數都將為構建工具正確轉義，總之 CMake 官方文件建議使用。


而上述指令大致上的意思就是說，在程式建置好後會輸入以下指令：
```bash=
$ cmake -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:${你的目標名稱}>/assets"
```
而這個指令的意思就是建立符號連結（Symbolic Link），如果是熟悉類 Unix 系統的使用者想必很熟悉，但 Windows 使用者可能就未必了，基本上這個東西跟 Windows 上的捷徑（`.lnk`）是截然不同的東西，功能雖然很像但基本上是完全不一樣，根據微軟官方的解釋，【符號連結是指向另一個檔案系統物件的檔案系統物件】，符號連結在檔案總管中會顯示為一般檔案或者是目錄，使用者或應用程式可以用完全相同的方式處理或是存取它們。
現在知道這個指令是在創建符號連結，但是路徑到底是哪裡呢？我們可以看出這邊有 CMake 的變數：`${CMAKE_CURRENT_SOURCE_DIR}` 和 `$<TARGET_FILE_DIR:目標名稱>`，前者為你當前 `CMakeLists.txt` 的所在根目錄，後者則是 [CMake 生產器表達式](https://cmake.org/cmake/help/v3.0/manual/cmake-generator-expressions.7.html)，它的語法其實是 `$<TARGET_FILE_DIR:tgt>`，這其中的 `tgt` 就是你專案的目標名稱，比如說 `add_executable(my-target)`，那 `my-target` 就是你的目標名稱，但如果你是有設定變數的話，例如 `set(MY_EXECUTABLE "my-target")`，那你的目標名稱反而要變成 `${MY_EXECUTABLE}`。
回到主題，`$<TARGET_FILE_DIR:目標名稱>` 回傳的就是你程式建置好的所在目錄（可以說是建置根目錄），所以我們只要把符號連結建立在該目錄下，你的外部檔案就可以被程式讀取到了。

CMake 可以透過參數 `-E` 來實現例如建立符號連結 `create_symlink`、複製資料夾 `copy_directory`、刪除檔案 `remove`、`cat` 和 `touch` 等行為。

### 錯誤排除
1. 如果你會發生以下錯誤的話：
   ![](https://i.imgur.com/MrdW3Qh.png)
   其解決方案很簡單，就是到【設定】->【更新與安全性】->【開發人員專用】，然後將【開發人員模式】給它開啟起來就解決了：
   ![](https://i.imgur.com/0exh51d.png)

2. 如果你會發生以下錯誤的話：
   ![](https://i.imgur.com/QL88khC.png)
   其解決方案很簡單，把整個建置資料夾刪除，並且也清除掉 CMake 快取，重新建立就可以解決了。


###### tags: `OpenGL`