# cJSON

一个超轻量级的 ANSI C JSON 解析器

## 许可证

MIT 许可证

>  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
>
>  特此免费授予任何获得本软件及相关文档文件（“软件”）副本的人士不受限制地处理本软件的权利，包括但不限于使用、复制、修改、合并、发布、分发、再许可和/或销售软件副本的权利，并允许向其提供本软件的人士这样做，但须遵守以下条件：
>
>  上述版权声明和本许可声明应包含在本软件的所有副本或主要部分中。
>
>  本软件按“原样”提供，不作任何明示或暗示的保证，包括但不限于对适销性、特定用途适用性和非侵权性的保证。在任何情况下，作者或版权持有人均不对任何索赔、损害赔偿或其他责任承担任何责任，无论是在合同诉讼、侵权行为还是其他方面，由本软件或本软件的使用或其他交易引起或与之相关。

## 用法

#### **欢迎使用 cJSON**

cJSON 旨在成为能够帮助您完成工作的最简单的解析器。
它仅由一个 C 源文件和一个头文件组成。

JSON 的最佳描述在这里：http://www.json.org/
它就像 XML，但没有多余的成分。您可以用它来移动数据、存储东西，或者只是
普遍地表示您程序的状态。

作为一个库，cJSON 的存在是为了尽可能地减少您的工作量，但又不会妨碍您。
从实用主义的角度（即忽略事实）来说，我认为您可以
在两种模式下使用它：自动模式和手动模式。让我们快速了解一下。

我从这个页面摘录了一些 JSON：http://www.json.org/fatfree.html
那个页面启发我编写了 cJSON，这是一个试图分享与 JSON 本身相同
理念的解析器。简单、直接、不碍事。

### 构建

有几种方法可以将 cJSON 集成到您的项目中。

#### 拷贝源代码

因为整个库只有一个 C 文件和一个头文件，您可以直接将 `cJSON.h` 和 `cJSON.c` 拷贝到您的项目源代码中并开始使用。

cJSON 是用 ANSI C (C89) 编写的，以支持尽可能多的平台和编译器。

#### CMake

通过 CMake，cJSON 支持一个功能齐全的构建系统。这样您可以获得最多的功能。支持等于或高于 2.8.5 版本的 CMake。使用 CMake 时，建议进行外部构建（out of tree build），这意味着编译后的文件会放在一个与源文件分开的目录中。因此，要在 Unix 平台上用 CMake 构建 cJSON，请创建一个 `build` 目录并在其中运行 CMake。

```
mkdir build
cd build
cmake ..
```

这将创建一个 Makefile 和一堆其他文件。然后您可以编译它：

```
make
```

如果您愿意，可以使用 `make install` 来安装它。默认情况下，它会将头文件安装到 `/usr/local/include/cjson`，库文件安装到 `/usr/local/lib`。它还会安装用于 pkg-config 的文件，以便更容易地检测和使用已安装的 CMake。并且它还会安装 CMake 配置文件，其他基于 CMake 的项目可以使用这些文件来发现这个库。

您可以通过传递给 CMake 的一系列不同选项来更改构建过程。使用 `On` 开启它们，使用 `Off` 关闭它们：

* `-DENABLE_CJSON_TEST=On`：启用测试构建。（默认开启）
* `-DENABLE_CJSON_UTILS=On`：启用 cJSON_Utils 构建。（默认关闭）
* `-DENABLE_TARGET_EXPORT=On`：启用 CMake 目标的导出。如果出现问题，请关闭它。（默认开启）
* `-DENABLE_CUSTOM_COMPILER_FLAGS=On`：启用自定义编译器标志（目前适用于 Clang、GCC 和 MSVC）。如果出现问题，请关闭它。（默认开启）
* `-DENABLE_VALGRIND=On`：使用 [valgrind](http://valgrind.org) 运行测试。（默认关闭）
* `-DENABLE_SANITIZERS=On`：在编译 cJSON 时启用 [AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer) 和 [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)（如果可能）。（默认关闭）
* `-DENABLE_SAFE_STACK`：启用 [SafeStack](https://clang.llvm.org/docs/SafeStack.html) 插桩。目前仅适用于 Clang 编译器。（默认关闭）
* `-DBUILD_SHARED_LIBS=On`：构建共享库。（默认开启）
* `-DBUILD_SHARED_AND_STATIC_LIBS=On`：同时构建共享库和静态库。（默认关闭）
* `-DCMAKE_INSTALL_PREFIX=/usr`：设置安装前缀。
* `-DENABLE_LOCALES=On`：启用 localeconv 方法的使用。（默认开启）
* `-DCJSON_OVERRIDE_BUILD_SHARED_LIBS=On`：允许使用 `-DCJSON_BUILD_SHARED_LIBS` 覆盖 `BUILD_SHARED_LIBS` 的值。
* `-DENABLE_CJSON_VERSION_SO`：启用 cJSON so 版本。（默认开启）

如果您正在为 Linux 发行版打包 cJSON，您可能会采取以下步骤：
```
mkdir build
cd build
cmake .. -DENABLE_CJSON_UTILS=On -DENABLE_CJSON_TEST=Off -DCMAKE_INSTALL_PREFIX=/usr
make
make DESTDIR=$pkgdir install
```

在 Windows 上，CMake 通常用于通过在 Visual Studio 的开发者命令提示符中运行来创建 Visual Studio 解决方案文件，具体步骤请遵循 CMake 和 Microsoft 的官方文档，并使用您选择的在线搜索引擎。上述选项的描述仍然普遍适用，尽管并非所有选项都在 Windows 上有效。

#### Makefile

**注意：** 此方法已弃用。请尽可能使用 CMake。Makefile 的支持仅限于修复错误。

如果您没有 CMake，但仍有 GNU make，您可以使用 makefile 来构建 cJSON：

在包含源代码的目录中运行此命令，它将自动编译静态库、共享库和一个小测试程序（不是完整的测试套件）。

```
make all
```

如果您愿意，可以使用 `make install` 将编译好的库安装到您的系统中。默认情况下，它会将头文件安装在 `/usr/local/include/cjson`，库文件安装在 `/usr/local/lib`。但您可以通过设置 `PREFIX` 和 `DESTDIR` 变量来更改此行为：`make PREFIX=/usr DESTDIR=temp install`。并使用以下命令卸载它们：`make PREFIX=/usr DESTDIR=temp uninstall`。

#### Meson

为了让 cjson 在使用 meson 的项目中工作，必须包含 libcjson 依赖项：

```meson
project('c-json-example', 'c')

cjson = dependency('libcjson')

example = executable(
    'example',
    'example.c',
    dependencies: [cjson],
)
```

#### Vcpkg

您可以使用 [vcpkg](https://github.com/Microsoft/vcpkg) 依赖管理器下载和安装 cJSON：
```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
vcpkg install cjson
```

vcpkg 中的 cJSON 端口由微软团队成员和社区贡献者保持更新。如果版本过时，请在 vcpkg 仓库中 [创建一个 issue 或 pull request](https://github.com/Microsoft/vcpkg)。

### 包含 cJSON

如果您通过 CMake 或 Makefile 安装了它，您可以像这样包含 cJSON：

```c
#include <cjson/cJSON.h>
```

### 数据结构

cJSON 使用 `cJSON` 结构体数据类型来表示 JSON 数据：

```c
/* cJSON 结构体：*/
typedef struct cJSON
{
    /* next/prev 构成了同级 cJSON 节点的双向链表 */
    struct cJSON *next;
    struct cJSON *prev;
    /* 对于数组或对象，child 指向其子 cJSON 节点链表的头节点 */
    struct cJSON *child;
    /* cJSON 节点的类型 */
    int type;
    /* 如果 cJSON 节点是字符串类型，valuestring 指向其值 */
    char *valuestring;
    /* 不推荐直接写入 valueint，应使用 cJSON_SetNumberValue 代替 */
    int valueint;
    /* 如果 cJSON 节点是数字类型，其值存储在 valuedouble 中 */
    double valuedouble;
    /* cJSON 节点在父对象中的键（key） */
    char *string;
} cJSON;
```

这种类型的项代表一个 JSON 值。类型存储在 `type` 中，以位标志（bit-flag）的形式存在（**这意味着您不能仅通过比较 `type` 的值来判断类型**）。

要检查一个项的类型，请使用相应的 `cJSON_Is...` 函数。它会先进行 `NULL` 检查，然后进行类型检查，如果项是该类型，则返回一个布尔值。

类型可以是以下之一：

* `cJSON_Invalid` (使用 `cJSON_IsInvalid` 检查)：表示一个不包含任何值的无效项。如果您将项的所有字节都设置为零，就会自动得到这个类型。
* `cJSON_False` (使用 `cJSON_IsFalse` 检查)：表示一个 `false`布尔值。您也可以使用 `cJSON_IsBool` 检查通用的布尔值。
* `cJSON_True` (使用 `cJSON_IsTrue` 检查)：表示一个 `true` 布尔值。您也可以使用 `cJSON_IsBool` 检查通用的布尔值。
* `cJSON_NULL` (使用 `cJSON_IsNull` 检查)：表示一个 `null` 值。
* `cJSON_Number` (使用 `cJSON_IsNumber` 检查)：表示一个数值。该值以 double 类型存储在 `valuedouble` 中，也存储在 `valueint` 中。如果数字超出了整数的范围，`valueint` 将使用 `INT_MAX` 或 `INT_MIN`。
* `cJSON_String` (使用 `cJSON_IsString` 检查)：表示一个字符串值。它以零结尾的字符串形式存储在 `valuestring` 中。
* `cJSON_Array` (使用 `cJSON_IsArray` 检查)：表示一个数组值。这是通过让 `child` 指向一个由 `cJSON` 项组成的链表来实现的，这些项代表数组中的值。元素之间通过 `next` 和 `prev` 连接，其中第一个元素的 `prev.next == NULL`，最后一个元素的 `next == NULL`。
* `cJSON_Object` (使用 `cJSON_IsObject` 检查)：表示一个对象值。对象的存储方式与数组相同，唯一的区别是对象中的项将其键（key）存储在 `string` 中。
* `cJSON_Raw` (使用 `cJSON_IsRaw` 检查)：表示任何以零结尾的字符数组形式存储的 JSON，存储在 `valuestring` 中。例如，这可以用来避免重复打印相同的静态 JSON 以节省性能。cJSON 在解析时绝不会创建这种类型。另请注意，cJSON 不会检查它是否是有效的 JSON。

此外，还有以下两个标志：

* `cJSON_IsReference`：指定 `child` 指向的项和/或 `valuestring` 不为此项所有，它只是一个引用。因此 `cJSON_Delete` 和其他函数只会释放此项，而不会释放其 `child`/`valuestring`。
* `cJSON_StringIsConst`：这意味着 `string` 指向一个常量字符串。这意味着 `cJSON_Delete` 和其他函数不会尝试释放 `string`。

### 使用数据结构

对于每种值类型，都有一个 `cJSON_Create...` 函数可以用来创建该类型的项。
所有这些函数都会分配一个 `cJSON` 结构体，稍后可以用 `cJSON_Delete` 删除。
请注意，您必须在某个时候删除它们，否则会导致内存泄漏。
**重要提示**：如果您已经将一个项添加到一个数组或对象中，您**绝对不能**用 `cJSON_Delete` 来删除它。将其添加到数组或对象中会转移其所有权，因此当该数组或对象被删除时，
它也会被一并删除。您也可以使用 `cJSON_SetValuestring` 来更改 `cJSON_String` 的 `valuestring`，您无需手动释放之前的 `valuestring`。

#### 基本类型

* **null** 使用 `cJSON_CreateNull` 创建
* **布尔值** 使用 `cJSON_CreateTrue`、`cJSON_CreateFalse` 或 `cJSON_CreateBool` 创建
* **数字** 使用 `cJSON_CreateNumber` 创建。这将同时设置 `valuedouble` 和 `valueint`。如果数字超出了整数的范围，`valueint` 将使用 `INT_MAX` 或 `INT_MIN`。
* **字符串** 使用 `cJSON_CreateString` (拷贝字符串) 或 `cJSON_CreateStringReference` (直接指向字符串，这意味着 `valuestring` 不会被 `cJSON_Delete` 删除，您需要负责其生命周期，这对于常量很有用) 创建。

#### 数组

您可以使用 `cJSON_CreateArray` 创建一个空数组。`cJSON_CreateArrayReference` 可用于创建一个不“拥有”其内容的数组，因此其内容不会被 `cJSON_Delete` 删除。

要向数组中添加项，请使用 `cJSON_AddItemToArray` 将项追加到末尾。
使用 `cJSON_AddItemReferenceToArray` 可以将一个元素作为对另一个项、数组或字符串的引用添加。这意味着 `cJSON_Delete` 不会删除该项的 `child` 或 `valuestring` 属性，因此如果它们已在别处使用，则不会发生双重释放。
要在中间插入项，请使用 `cJSON_InsertItemInArray`。它将在给定的从 0 开始的索引处插入一个项，并将所有现有项向右移动。

如果您想从数组的给定索引处取出一个项并继续使用它，请使用 `cJSON_DetachItemFromArray`，它将返回被分离的项，所以请确保将其赋给一个指针，否则您将会有内存泄漏。

删除项是使用 `cJSON_DeleteItemFromArray` 完成的。它的工作方式类似于 `cJSON_DetachItemFromArray`，但会通过 `cJSON_Delete` 删除被分离的项。

您也可以就地替换数组中的项。可以使用 `cJSON_ReplaceItemInArray` 按索引替换，或者使用 `cJSON_ReplaceItemViaPointer` 给定一个指向元素的指针。如果失败，`cJSON_ReplaceItemViaPointer` 将返回 `0`。其内部操作是分离旧项，删除它，然后在其位置插入新项。

要获取数组的大小，请使用 `cJSON_GetArraySize`。使用 `cJSON_GetArrayItem` 来获取给定索引处的元素。

因为数组是以链表形式存储的，所以通过索引迭代效率很低 (`O(n²)`），因此您可以使用 `cJSON_ArrayForEach` 宏以 `O(n)` 的时间复杂度遍历数组。

#### 对象

您可以使用 `cJSON_CreateObject` 创建一个空对象。`cJSON_CreateObjectReference` 可用于创建一个不“拥有”其内容的对象，因此其内容不会被 `cJSON_Delete` 删除。

要向对象添加项，请使用 `cJSON_AddItemToObject`。使用 `cJSON_AddItemToObjectCS` 向对象添加一个名称为常量或引用的项（项的键，即 `cJSON` 结构中的 `string`），这样它就不会被 `cJSON_Delete` 释放。
使用 `cJSON_AddItemReferenceToArray` 可以将一个元素作为对另一个对象、数组或字符串的引用添加。这意味着 `cJSON_Delete` 不会删除该项的 `child` 或 `valuestring` 属性，因此如果它们已在别处使用，则不会发生双重释放。

如果您想从对象中取出一个项，请使用 `cJSON_DetachItemFromObjectCaseSensitive`，它将返回被分离的项，所以请确保将其赋给一个指针，否则您将会有内存泄漏。

删除项是使用 `cJSON_DeleteItemFromObjectCaseSensitive` 完成的。它的工作方式类似于 `cJSON_DetachItemFromObjectCaseSensitive` 加上 `cJSON_Delete`。

您也可以就地替换对象中的项。可以使用 `cJSON_ReplaceItemInObjectCaseSensitive` 按键替换，或者使用 `cJSON_ReplaceItemViaPointer` 给定一个指向元素的指针。如果失败，`cJSON_ReplaceItemViaPointer` 将返回 `0`。其内部操作是分离旧项，删除它，然后在其位置插入新项。

要获取对象的大小，您可以使用 `cJSON_GetArraySize`，这是因为对象内部也是作为数组存储的。

如果您想访问对象中的项，请使用 `cJSON_GetObjectItemCaseSensitive`。

要遍历对象，您可以使用 `cJSON_ArrayForEach` 宏，方法与数组相同。

cJSON 还提供了方便的辅助函数，用于快速创建新项并将其添加到对象中，例如 `cJSON_AddNullToObject`。它们会返回指向新项的指针，如果失败则返回 `NULL`。

### 解析 JSON

给定一个以零结尾的字符串中的 JSON，您可以使用 `cJSON_Parse` 进行解析。

```c
cJSON *json = cJSON_Parse(string);
```

给定一个字符串中的 JSON（无论是否以零结尾），您可以使用 `cJSON_ParseWithLength` 进行解析。

```c
cJSON *json = cJSON_ParseWithLength(string, buffer_length);
```

它将解析 JSON 并分配一个代表它的 `cJSON` 项树。一旦返回，您需要完全负责在使用后用 `cJSON_Delete` 释放它。

`cJSON_Parse` 使用的分配器默认是 `malloc` 和 `free`，但可以通过 `cJSON_InitHooks` 进行（全局）更改。

如果发生错误，可以使用 `cJSON_GetErrorPtr` 访问输入字符串中错误位置的指针。但请注意，这在多线程场景中可能会产生竞争条件，在这种情况下，最好使用带有 `return_parse_end` 的 `cJSON_ParseWithOpts`。
默认情况下，输入字符串中跟随在已解析 JSON 之后的多余字符不会被视为错误。

如果您需要更多选项，请使用 `cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated)`。
`return_parse_end` 返回一个指向输入字符串中 JSON 结尾的指针，或者发生错误的位置的指针（从而以线程安全的方式替代 `cJSON_GetErrorPtr`）。`require_null_terminated`，如果设置为 `1`，则如果输入字符串在 JSON 之后还包含数据，则会报错。

如果您需要更多关于缓冲区长度的选项，请使用 `cJSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, cJSON_bool require_null_terminated)`。

### 打印 JSON

给定一个 `cJSON` 项树，您可以使用 `cJSON_Print` 将它们打印为字符串。

```c
char *string = cJSON_Print(json);
```

它将分配一个字符串并将树的 JSON 表示打印到其中。一旦返回，您需要完全负责在使用后用您的分配器释放它。（通常是 `free`，取决于用 `cJSON_InitHooks` 设置的内容）。

`cJSON_Print` 将打印带空格的格式化文本。如果您想打印不带格式的文本，请使用 `cJSON_PrintUnformatted`。

如果您大致知道结果字符串的大小，可以使用 `cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt)`。`fmt` 是一个布尔值，用于开关带空格的格式化。`prebuffer` 指定用于打印的初始缓冲区大小。`cJSON_Print` 目前为其初始缓冲区使用 256 字节。一旦打印空间不足，将分配一个新缓冲区，并将旧缓冲区的内容复制过来，然后继续打印。

通过使用 `cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format)`，可以完全避免这些动态缓冲区分配。它接受一个指向要打印到的缓冲区的指针及其长度。如果达到长度限制，打印将失败并返回 `0`。如果成功，则返回 `1`。请注意，您应该提供比实际需要多 5 个字节的空间，因为 cJSON 在估算所提供内存是否足够时并非 100% 准确。

### 示例

在此示例中，我们将构建并解析以下 JSON：

```json
{
    "name": "Awesome 4K",
    "resolutions": [
        {
            "width": 1280,
            "height": 720
        },
        {
            "width": 1920,
            "height": 1080
        },
        {
            "width": 3840,
            "height": 2160
        }
    ]
}
```

#### 打印

让我们构建上面的 JSON 并将其打印成字符串：

```c
// 创建一个带有一系列支持分辨率的显示器
// 注意：返回一个在堆上分配的字符串，您需要在用完后释放它。
char *create_monitor(void)
{
    const unsigned int resolution_numbers[3][2] = {
        {1280, 720},
        {1920, 1080},
        {3840, 2160}
    };
    char *string = NULL;
    cJSON *name = NULL;
    cJSON *resolutions = NULL;
    cJSON *resolution = NULL;
    cJSON *width = NULL;
    cJSON *height = NULL;
    size_t index = 0;

    cJSON *monitor = cJSON_CreateObject();
    if (monitor == NULL)
    {
        goto end;
    }

    name = cJSON_CreateString("Awesome 4K");
    if (name == NULL)
    {
        goto end;
    }
    /* 创建成功后，立即将其添加到 monitor 对象中，
     * 从而将指针的所有权转移给它 */
    cJSON_AddItemToObject(monitor, "name", name);

    resolutions = cJSON_CreateArray();
    if (resolutions == NULL)
    {
        goto end;
    }
    cJSON_AddItemToObject(monitor, "resolutions", resolutions);

    for (index = 0; index < (sizeof(resolution_numbers) / (2 * sizeof(int))); ++index)
    {
        resolution = cJSON_CreateObject();
        if (resolution == NULL)
        {
            goto end;
        }
        cJSON_AddItemToArray(resolutions, resolution);

        width = cJSON_CreateNumber(resolution_numbers[index][0]);
        if (width == NULL)
        {
            goto end;
        }
        cJSON_AddItemToObject(resolution, "width", width);

        height = cJSON_CreateNumber(resolution_numbers[index][1]);
        if (height == NULL)
        {
            goto end;
        }
        cJSON_AddItemToObject(resolution, "height", height);
    }

    string = cJSON_Print(monitor);
    if (string == NULL)
    {
        fprintf(stderr, "打印 monitor 失败。\n");
    }

end:
    cJSON_Delete(monitor);
    return string;
}
```

或者，我们可以使用 `cJSON_Add...ToObject` 辅助函数来让我们的工作更简单一些：

```c
// 注意：返回一个在堆上分配的字符串，您需要在用完后释放它。
char *create_monitor_with_helpers(void)
{
    const unsigned int resolution_numbers[3][2] = {
        {1280, 720},
        {1920, 1080},
        {3840, 2160}
    };
    char *string = NULL;
    cJSON *resolutions = NULL;
    size_t index = 0;

    cJSON *monitor = cJSON_CreateObject();

    if (cJSON_AddStringToObject(monitor, "name", "Awesome 4K") == NULL)
    {
        goto end;
    }

    resolutions = cJSON_AddArrayToObject(monitor, "resolutions");
    if (resolutions == NULL)
    {
        goto end;
    }

    for (index = 0; index < (sizeof(resolution_numbers) / (2 * sizeof(int))); ++index)
    {
        cJSON *resolution = cJSON_CreateObject();

        if (cJSON_AddNumberToObject(resolution, "width", resolution_numbers[index][0]) == NULL)
        {
            goto end;
        }

        if (cJSON_AddNumberToObject(resolution, "height", resolution_numbers[index][1]) == NULL)
        {
            goto end;
        }

        cJSON_AddItemToArray(resolutions, resolution);
    }

    string = cJSON_Print(monitor);
    if (string == NULL)
    {
        fprintf(stderr, "打印 monitor 失败。\n");
    }

end:
    cJSON_Delete(monitor);
    return string;
}
```

#### 解析

在此示例中，我们将解析上述格式的 JSON，并检查显示器是否支持全高清分辨率，同时打印一些诊断输出：

```c
/* 如果显示器支持全高清则返回 1，否则返回 0 */
int supports_full_hd(const char * const monitor)
{
    const cJSON *resolution = NULL;
    const cJSON *resolutions = NULL;
    const cJSON *name = NULL;
    int status = 0;
    cJSON *monitor_json = cJSON_Parse(monitor);
    if (monitor_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "错误发生在：%s\n", error_ptr);
        }
        status = 0;
        goto end;
    }

    name = cJSON_GetObjectItemCaseSensitive(monitor_json, "name");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        printf("正在检查显示器 \"%s\"\n", name->valuestring);
    }

    resolutions = cJSON_GetObjectItemCaseSensitive(monitor_json, "resolutions");
    cJSON_ArrayForEach(resolution, resolutions)
    {
        cJSON *width = cJSON_GetObjectItemCaseSensitive(resolution, "width");
        cJSON *height = cJSON_GetObjectItemCaseSensitive(resolution, "height");

        if (!cJSON_IsNumber(width) || !cJSON_IsNumber(height))
        {
            status = 0;
            goto end;
        }

        if ((width->valuedouble == 1920) && (height->valuedouble == 1080))
        {
            status = 1;
            goto end;
        }
    }

end:
    cJSON_Delete(monitor_json);
    return status;
}
```

请注意，除了 `cJSON_Parse` 的结果外，没有进行 NULL 检查，因为 `cJSON_GetObjectItemCaseSensitive` 已经检查了 `NULL` 输入，所以 `NULL` 值只是被传播下去，而 `cJSON_IsNumber` 和 `cJSON_IsString` 在输入为 `NULL` 时会返回 `0`。

### 注意事项

#### 零字符

cJSON 不支持包含零字符 `'\0'` 或 `\u0000` 的字符串。由于字符串是以零结尾的，目前的 API 无法实现这一点。

#### 字符编码

cJSON 仅支持 UTF-8 编码的输入。在大多数情况下，它不会拒绝无效的 UTF-8 作为输入，而只是将其原样传递。只要输入不包含无效的 UTF-8，输出将始终是有效的 UTF-8。

#### C 标准

cJSON 是用 ANSI C (或 C89, C90) 编写的。如果您的编译器或 C 库不遵循此标准，则不能保证其行为正确。

注意：ANSI C 不是 C++，因此不应使用 C++ 编译器进行编译。但是，您可以使用 C 编译器编译它，然后与您的 C++ 代码链接。尽管使用 C++ 编译器编译可能有效，但不能保证其行为正确。

#### 浮点数

cJSON 官方不支持除 IEEE754 双精度浮点数之外的任何 `double` 实现。它可能仍然适用于其他实现，但与这些实现相关的错误将被视为无效。

cJSON 目前支持的浮点数字面量的最大长度为 63 个字符。

#### 数组和对象的深度嵌套

cJSON 不支持嵌套过深的数组和对象，因为这会导致堆栈溢出。为防止这种情况，cJSON 将深度限制为 `CJSON_NESTING_LIMIT`，默认为 1000，但可以在编译时更改。

#### 线程安全

通常情况下，cJSON **不是线程安全的**。

但在以下条件下，它是线程安全的：

* 从不使用 `cJSON_GetErrorPtr`（可以使用 `cJSON_ParseWithOpts` 的 `return_parse_end` 参数代替）。
* `cJSON_InitHooks` 仅在任何线程中使用 cJSON 之前调用。
* 在所有对 cJSON 函数的调用返回之前，从不调用 `setlocale`。

#### 大小写敏感性

cJSON 最初创建时，并未遵循 JSON 标准，不区分大小写字母。如果您想要正确、符合标准的行为，您需要在可用之处使用 `CaseSensitive` 函数。

#### 重复的对象成员

cJSON 支持解析和打印包含具有相同名称的多个成员的对象。然而，`cJSON_GetObjectItemCaseSensitive` 将始终只返回第一个。

# 尽情使用 cJSON！

- Dave Gamble (原作者)
- Max Bruckner 和 Alan Wang (当前维护者)
- 以及其他 [cJSON 贡献者](CONTRIBUTORS.md)



---