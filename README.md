# Battle of the Flatland (BOTF)

Скелет client–server 2D-игры на C++20 + CMake + SFML + Protobuf + spdlog.

Репозиторий: https://github.com/OleksiiShevtsov/BOTF

## Раскладка папок

```
<parent>/
├── BOTF/     ← этот репозиторий (исходники)
├── LIB/      ← SFML, googletest, protobuf, spdlog
└── BUILD/    ← CMAKE_BUILDS_ROOT указывает сюда
```

## Окружение

Глобально в системе (как в CrashCourse):

- `CMAKE_BUILDS_ROOT` → папка `BUILD` (рядом с `LIB` и `BOTF`)

Опционально:

- `BOTF_LIB_ROOT` → папка `LIB` (если CMake/setup не находят её сами)

## CMakePresets

Как в остальных уроках — только через env, без абсолютных путей:

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "debug",
      "binaryDir": "$env{CMAKE_BUILDS_ROOT}/${sourceDirName}/debug"
    },
    {
      "name": "release",
      "binaryDir": "$env{CMAKE_BUILDS_ROOT}/${sourceDirName}/release"
    }
  ]
}
```

В Visual Studio выбери preset **debug** или **release**.

## LIB

```bat
setup.bat
```

Ищет `LIB` рядом с `BOTF` (или `BOTF_LIB_ROOT`).  
Если SFML / googletest / protobuf / spdlog уже есть — **не клонирует**.  
Protobuf: тег `v21.12` в `%LIB%\protobuf` (битую папку без `CMakeLists.txt` переклонирует).  
CMake на Windows/LIB собирает protobuf из исходников (`protobuf_BUILD_TESTS=OFF`), без системного `find_package`.  
spdlog: `LIB/spdlog` → цветная консоль через `botf::log`.  
После первого clone **запусти `setup.bat`** — иначе CMake не найдёт зависимости.

## Logging

`botf::log::{info,warn,error,debug}` в `common/sources/include/common/log.hpp` — обёртка над spdlog.  
Только цветной stdout (`stdout_color_mt`), без файлов.  
Логируем listen / connect / disconnect / ошибки envelope / start-stop server/client.  
**Не** логируем Move/WorldState каждый тик и ничего из `logic/`.

## Style

Члены класса: `m_name` (не `name_`). Константы: `kName`.  
Подробнее: [`docs/STYLE.md`](docs/STYLE.md).

## Format

В корне лежит `.clang-format` (стиль Google, отступ — таб width 4, колонка 100).  
Visual Studio подхватывает его для **Format Document**.

При обычной сборке CMake сначала гоняет `clang-format` (target `botf_format`), потом компилирует.  
Только файлы проекта: `*.cpp` / `*.h` / `*.hpp` в `client/`, `server/`, `net/`, `common/sources/`, `tests/`.  
Не трогает `LIB`, `.proto` и сгенерированные `*.pb.cc`.  
Нужен `clang-format` в PATH. Отключить:

```bat
cmake --preset debug -DBOTF_FORMAT_ON_BUILD=OFF
```

Вручную:

```bat
format.bat
cmake --build --preset debug --target botf_format
```

## Build

```bat
build.bat
build.bat release
```

или:

```bat
cmake --preset debug
cmake --build --preset debug
```

Бинарники: `botf_server`, `botf_client` в `%CMAKE_BUILDS_ROOT%\BOTF\debug` (или `release`), если папка репозитория называется `BOTF`.

## Транспорт

```
[uint32 LE length][serialized botf.Envelope]
```

- **SFML Network** — доставка байтов  
- **Protobuf** — тело пакета  
- framing: `net/include/net/packet_framing.hpp`

## Модули

```
BOTF/
├── cmake/
├── common/
│   ├── sources/                  # types, DTO, botf::log (spdlog)
│   └── protobuf/messages/        # .proto codegen
├── protocol/            # Envelope oneof codec, dispatch registry, proto ↔ DTO
├── net/                 # TcpServer, TcpClient, framing, queues (no protobuf)
├── server/
│   ├── engine/          # ServerEngine (tick loop)
│   └── logic/           # GameLogic / DynamicObject (no *.pb.h)
├── client/
│   └── app/             # ClientApp, AppRendering, Match (GameMap + Scene/IViewObject) (no *.pb.h)
└── tests/               # gtest: helpers + smoke (свой main.cpp)
```

`main` тонкий: только runtime (`ServerEngine` / `ClientApp`). Smoke живёт в `tests/`.

```
ClientConnect → spawn/bind figure → ClientConnect ack (object_id)
ClientInput(dx,dy,aim,fire) → net → GameLogic (move + aim + optional bullet spawn)
  → tick (figures, aims, bullets; prune dead) → WorldSnapshot → Scene (FigureView / BulletView)
```

Player hitbox (server): `ObjectHitbox` on `FigureObject` (default stub = `hitboxSize`×`hitboxSize`
from GlobalSettings via `installObjectHitbox()`); AABB centered on entity pos; does not rotate
with aim. Hit-test in `logic/hitbox.h` reads hitbox from the target (point / segment ∩ AABB).

HP + LifeStatus (`Alive` / `Dead` / `Waiting`): server authority; synced in WorldSnapshot.
Dead → no Move/Fire; figure stays until client disconnect; client draws RGB(40,40,40).
Local HP bar — draft `InformationPanel` (bottom-right).
Bullet hit (server): point ∩ AABB → `bulletDamage`, destroy bullet; no self-hit. Hit flash — 42.

Movement (server): incremental `v += a·dt`, vector clamp `|v| ≤ speedLimit`, `pos += v·dt`;
friction with zero-cross (no reverse). Constants `playerAcceleration` / `playerFriction` in GlobalSettings.

### Новая нотификация (checklist)

Кратко:

1. Proto + поле в `Envelope.body` oneof.
2. DTO + mappers + `MessageBody` + `encodeX` / decode case.
3. Отправитель → `OutboundNotification`; получатель → ветка в `protocol::dispatch`.
4. Net не трогать.

Полный гайд (поток, направления, когда не плодить msg, константы, пример): [`docs/ADD_NOTIFICATION.md`](docs/ADD_NOTIFICATION.md).

## Tests

Нужен GoogleTest (`libgtest-dev` / `LIB/googletest`). Свой entrypoint: `tests/main.cpp`, helpers в `tests/helpers/`.

```bat
cmake --build --preset debug --target tests
ctest --test-dir %CMAKE_BUILDS_ROOT%\BOTF\debug --output-on-failure
```

или напрямую:

```bat
%CMAKE_BUILDS_ROOT%\BOTF\debug\tests\tests.exe
```

Linux:

```bash
cmake --build "$CMAKE_BUILDS_ROOT/BOTF/debug" --target tests
ctest --test-dir "$CMAKE_BUILDS_ROOT/BOTF/debug" --output-on-failure
```
