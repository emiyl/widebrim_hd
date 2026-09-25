# `RoomObject::AddTextObj` Reverse-Engineering Notes

## Overview

`RoomObject::AddTextObj` creates and registers an `oButtonObject` associated with a room text object. It does not directly contain or assign the displayed text. Instead, it stores metadata in `RoomObject` arrays that is later consumed by `RoomObject::DoTextObj`.

The text is ultimately loaded from a file such as:

```
/room/tobj/t_42.txt
```

and passed to `FontObject::StringScreenCenter` for rendering.

## 1. `RoomObject::AddTextObj`

Decompiled function:

C++

```
void RoomObject::AddTextObj(
    int param_1,
    int x,
    int y,
    int width,
    int height,
    int param_6,
    int param_7
)
{
    ulong uVar1;
    oButtonObject *this;

    uVar1 = (ulong)(uint)param_1;

    if (*(int *)(uVar1 + 0x228) < 0x10) {
        this = operator.new(0x40);
        oButtonObject::oButtonObject(this);

        *(oButtonObject **)
            (uVar1 + 0x158 + (long)*(int *)(uVar1 + 0x228) * 8) = this;

        oButtonObject::SetupNoGfx(
            *(oButtonObject **)
                (uVar1 + 0x158 + (long)*(int *)(uVar1 + 0x228) * 8),
            y,
            width,
            height,
            param_6
        );

        *(char *)
            (uVar1 + (long)*(int *)(uVar1 + 0x228) + 0x1d8) = (char)x;

        *(int *)
            (uVar1 + (long)*(int *)(uVar1 + 0x228) * 4 + 0x1e8) = param_7;

        *(int *)(uVar1 + 0x228) =
            *(int *)(uVar1 + 0x228) + 1;
    }

    return;
}
```

### Important decompiler correction

The first parameter is declared as `int`, but it is immediately treated as an address and dereferenced at offsets such as `0x228`. It is almost certainly a `RoomObject *`.

The local variable named `this` is actually the newly allocated `oButtonObject`; the decompiler reused the name and made the output confusing.

A more accurate conceptual signature is:

C++

```
void RoomObject::AddTextObj(
    RoomObject *room,
    int x,
    int y,
    int width,
    int height,
    int param_6,
    int param_7
);
```

## 2. Object allocation and limit

The function checks:

C++

```
*(int *)(room + 0x228) < 0x10
```

This limits the number of registered text/button objects to 16 (`0x10`).

When the limit has not been reached, it:

1. Allocates `0x40` bytes for an `oButtonObject`.

2. Calls the `oButtonObject` constructor.

3. Stores the pointer in an array beginning at `RoomObject + 0x158`.

4. Initializes the object with `SetupNoGfx`.

5. Stores two pieces of metadata in arrays at `RoomObject + 0x1D8` and `RoomObject + 0x1E8`.

6. Increments the object count at `RoomObject + 0x228`.

The object pointer array is indexed as:

C++

```
room->field_158[index]
```

with 8-byte pointer slots.

## 3. `oButtonObject::SetupNoGfx`

Decompiled function:

C++

```
void oButtonObject::SetupNoGfx(
    oButtonObject *this,
    int param_1,
    int param_2,
    int param_3,
    int param_4
)
{
    *(int *)(this + 0x10) = param_1;
    *(int *)(this + 0x14) = param_2;
    *(undefined2 *)(this + 0x29) = 0;
    *(int *)(this + 0x18) = param_3;
    *(int *)(this + 0x1c) = param_4;
}
```

`AddTextObj` calls it as:

C++

```
oButtonObject::SetupNoGfx(button, y, width, height, param_6);
```

Therefore, the assignments are:

|
`oButtonObject` offset

|

Assigned value

|
| --- | --- |
|

`+0x10`

|

`y`

|
|

`+0x14`

|

`width`

|
|

`+0x18`

|

`height`

|
|

`+0x1C`

|

`param_6`

|
|

`+0x29`

|

`0` as a 16-bit value

|

This function does not receive a text string and does not directly load text. It appears to initialize geometry/configuration/state for the button-like object.

The `oButtonObject` allocation size is `0x40` bytes, so the constructor and other methods should be examined for the remaining fields, particularly offsets `+0x20` through `+0x3C`.

## 4. Metadata stored by `AddTextObj`

Let `index` be the current object count read from `RoomObject + 0x228`.

### First metadata array

C++

```
*(char *)(room + 0x1D8 + index) = (char)x;
```

This stores `x` as a single byte.

It is later read by `DoTextObj` as:

C++

```
this[(long)param_1 + 0x1D8]
```

and passed to a virtual function at vtable offset `0x50` for the normal (`param_1 >= 0`) branch.

The exact meaning is not fully confirmed, but it is likely a resource, character, or selection identifier rather than screen X coordinate. The value is truncated to one byte when stored.

### Second metadata array

C++

```
*(int *)(room + 0x1E8 + index * 4) = param_7;
```

This stores `param_7` as a 32-bit integer.

`DoTextObj` reads it as:

C++

```
*(undefined4 *)(room + 0x1E8 + param_1 * 4)
```

and uses it to construct the text filename:

```
/room/tobj/t_<param_7>.txt
```

For example, if `param_7 == 42`, the path becomes:

```
/room/tobj/t_42.txt
```

This makes `param_7` the likely text-file/message identifier.

## 5. `RoomObject::DoTextObj`

`DoTextObj` consumes the metadata populated by `AddTextObj`.

Its signature is:

C++

```
void RoomObject::DoTextObj(RoomObject *this, int param_1, int param_2);
```

The relevant normal branch (`param_1 >= 0`) is:

C++

```
(**(code **)(**(long **)(this + 0x110) + 0x50))
    (*(long **)(this + 0x110), this[(long)param_1 + 0x1d8]);

FUN_002c7d38(
    &TmpStr,
    0x800,
    "/room/tobj/t_%i.txt",
    *(undefined4 *)(this + (long)param_1 * 4 + 0x1e8)
);
```

Conceptually:

C++

```
resource_select(
    room->field_110,
    room->text_meta_byte_1d8[param_1]
);

format(
    TmpStr,
    "/room/tobj/t_%i.txt",
    room->text_file_id_1e8[param_1]
);
```

Then it loads and displays the selected text.

### Special cases

#### `param_1 == -1`

C++

```
resource_select(room->field_110, 3);
format(TmpStr, "/room/tobj/t_0.txt");
```

This selects resource ID `3` and loads:

```
/room/tobj/t_0.txt
```

#### `param_1 == -2`

C++

```
format(acStack_458, 0x40, "charm_%02d", param_2);

resource_select_other(room->field_110, acStack_458);

pcVar3 = "/room/tobj/t_200.txt";
format(TmpStr, "/room/tobj/t_200.txt");
```

This formats a string such as:

```
charm_01
charm_02
charm_15
```

and passes it to a virtual function at vtable offset `0x58`. It then loads:

```
/room/tobj/t_200.txt
```

The exact purpose of the `charm_%02d` string and the virtual functions at offsets `0x50` and `0x58` requires further reverse engineering.

## 6. Actual text loading

`DoTextObj` uses:

C++

```
char acStack_458[1024];
```

It eventually calls:

C++

```
SystemObject::LoadText(
    (SystemObject *)&systemDS,
    &TmpStr,
    acStack_458
);
```

The decompiled `LoadText` function is:

C++

```
void __thiscall SystemObject::LoadText(
    SystemObject *this,
    char *param_1,
    char *param_2
)
{
    long lVar1;
    char *pcVar2;
    void *__src;
    uint local_3c;
    long local_38;

    lVar1 = tpidr_el0;
    local_38 = *(long *)(lVar1 + 0x28);

    memset(FileBuffer, 0, 0x50000);
    local_3c = 0;

    pcVar2 = (char *)LoadFileToMem(
        (SystemObject *)&systemDS,
        param_1,
        FileBuffer,
        &local_3c
    );

    if (pcVar2 == (char *)0x0) {
        FUN_002d05c4(param_2, 0xffffffffffffffff);
    }
    else {
        __src = (void *)ConvertFont(pcVar2);
        memcpy(param_2, __src, (ulong)(local_3c + 1));
    }

    if (*(long *)(lVar1 + 0x28) == local_38) {
        return;
    }

    __stack_chk_fail(pcVar2 != (char *)0x0);
}
```

Conceptually:

1. Clear global `FileBuffer` (`0x50000` bytes).

2. Load the file named by `param_1` into `FileBuffer`.

3. Store the loaded size in `local_3c`.

4. If loading fails, call `FUN_002d05c4` on the output buffer.

5. If loading succeeds, pass the loaded data to `ConvertFont`.

6. Copy `local_3c + 1` bytes from the converted result into `param_2`.

In `DoTextObj`, `param_2` is `acStack_458`, so the converted result is copied into the local text buffer.

### Important uncertainty

The exact behavior of `ConvertFont` is unknown. It may perform an encoding conversion, custom character mapping, font-specific conversion, or another transformation. Do not assume the file is UTF-8 until `ConvertFont` is inspected.

The exact contract of `LoadFileToMem` is also unknown. Inspect it to determine:

* Whether `local_3c` is a byte count.

* Whether the source is a normal file or an archive/resource system.

* Whether the loaded buffer is null-terminated.

* Whether the return value points into `FileBuffer`.

### Potential buffer-size issue

`LoadText` copies `local_3c + 1` bytes into the destination without receiving a destination capacity:

C++

```
memcpy(param_2, __src, (ulong)(local_3c + 1));
```

`DoTextObj` passes a 1024-byte stack buffer:

C++

```
char acStack_458[1024];
```

If the converted content exceeds 1024 bytes, this may overflow the destination. The actual safety depends on constraints elsewhere in the original program.

## 7. Rendering the loaded text

After loading, `DoTextObj` calls:

C++

```
FontObject::StringScreenCenter(
    (FontObject *)&font,
    0x10e,
    acStack_458,
    true,
    3,
    -0xf6,
    false,
    *(CRubi **)(this + 0x770)
);
```

The `acStack_458` buffer is therefore passed directly to the text rendering function.

The exact meanings of the other arguments should be confirmed by inspecting `StringScreenCenter`, but the important relationship is:

```
text file
    -> LoadFileToMem
    -> ConvertFont
    -> memcpy into acStack_458
    -> StringScreenCenter(acStack_458, ...)
```

## 8. High-level flow

```
RoomObject::AddTextObj
    |
    | allocate oButtonObject (0x40 bytes)
    | call constructor
    | store pointer at RoomObject + 0x158 + index * 8
    | SetupNoGfx(button, y, width, height, param_6)
    | store (char)x at RoomObject + 0x1D8 + index
    | store param_7 at RoomObject + 0x1E8 + index * 4
    | increment object count at RoomObject + 0x228
    |
    v
RoomObject::DoTextObj(index, ...)
    |
    | read metadata at +0x1D8 and +0x1E8
    | select a resource through RoomObject + 0x110
    | construct "/room/tobj/t_<param_7>.txt"
    |
    v
SystemObject::LoadText
    |
    | clear FileBuffer
    | LoadFileToMem(path, FileBuffer, &size)
    | ConvertFont(loaded_data)
    | copy converted data into acStack_458
    |
    v
FontObject::StringScreenCenter
    |
    v
Text displayed on screen
```

## 9. Recommended next reverse-engineering targets

1. `ConvertFont`

   * Determine the text encoding and character mapping.

   * Determine whether it allocates memory or returns a pointer into existing data.

   * Determine whether it adds or expects a null terminator.
2. `LoadFileToMem`

   * Confirm the file source and size semantics.

   * Determine whether paths are resolved through an archive/resource layer.

   * Confirm buffer termination and failure behavior.
3. `oButtonObject` constructor

   * Identify the remaining fields in the 0x40-byte object.

   * Check for text pointers, message IDs, rendering state, or geometry defaults.
4. `FontObject::StringScreenCenter`

   * Confirm the text argument and rendering parameters.

   * Determine whether it expects converted/custom-encoded text.
5. Virtual function at `RoomObject + 0x110`, vtable offset `0x50`

   * Determine the meaning of the first metadata byte stored at `+0x1D8`.
6. Virtual function at `RoomObject + 0x110`, vtable offset `0x58`

   * Determine the purpose of the `charm_%02d` identifier in the `param_1 == -2` path.

## Bottom line

`AddTextObj` does not store the literal text. It registers an `oButtonObject` and stores metadata:

* `x` is stored as a byte at `RoomObject + 0x1D8 + index`.

* `param_7` is stored as a 32-bit file/message ID at `RoomObject + 0x1E8 + index * 4`.

* `param_7` is later used by `DoTextObj` to build `/room/tobj/t_<param_7>.txt`.

* `LoadText` loads that file, passes it through `ConvertFont`, and copies the result into the text buffer.

* `StringScreenCenter` receives the resulting buffer for display.

The exact semantics of the metadata byte, `param_6`, `ConvertFont`, and the resource-selection virtual functions remain to be established.
