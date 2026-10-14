# BOTF code style

## Naming

| Что | Стиль | Пример |
|-----|--------|--------|
| Члены класса | `m_` + camelCase | `m_position`, `m_latestWorld` |
| Локальные / параметры | camelCase без префикса | `clientId`, `tickStart` |
| Свободные функции / методы | camelCase | `sceneFromWorldState`, `buildWorldState` |
| Константы | `k` + PascalCase | `kTickHz`, `kPort` |
| Типы / классы | PascalCase | `GameLogic`, `TcpServer` |
| Namespaces | `botf::…` | `botf::net`, `botf::logic`, `botf::app` |

Не использовать trailing underscore (`foo_`) для members.

Исключения (оставляем как есть):

- идентификаторы STL / языка (`static_cast`, `push_back`, `std::unordered_map`, …)
- сгенерированный protobuf API (`set_tick`, `players_size`, `mutable_position`, …)

> `clang-format` правит только пробелы/переносы; переименование идентификаторов — вручную / рефакторингом по этой таблице.

## .cpp definitions

В `.cpp` **не** оборачиваем translation unit в `namespace botf::… { }`.  
Out-of-line definitions — с полным квалификатором. File-local helpers — в безымянном `namespace` на уровне файла.

```cpp
// headers: namespace botf::net { class TcpClient; } — как раньше

namespace {
void appendBytes(...);
}  // namespace

botf::net::TcpClient::TcpClient(ClientInboundQueue& inbound, OutboundQueue& outbound)
    : m_inbound(inbound), m_outbound(outbound) {}

// return type вне class-scope lookup — квалифицируем явно:
botf::common::ID botf::DynamicObject::id() const { return m_id; }
```

Не использовать `using namespace botf::…` в `.cpp`. Generated `*.pb.cc` не трогаем.

## Форматирование

Корень проекта: `.clang-format` (Google-based, **табы** width 4, column 100).  
При сборке target `botf_format` прогоняет `*.cpp` / `*.h` / `*.hpp` модулей проекта.

```bash
./format.sh
# или
cmake --build build --target botf_format
```

## Слои (ориентир)

- `net/` — транспорт (`TcpServer` / `TcpClient`, framing, queues), без protobuf  
- `protocol/` — Envelope codec + MessageType factory + mappers proto ↔ DTO  
- `common/` — shared types + DTO (`MoveCommand`, `WorldSnapshot`, …)  
- `logic/` — симуляция, без SFML Network / `*.pb.h`  
- `ServerEngine` — серверный loop (tick + broadcast)  
- `ClientApp` — клиентский loop (input + net + scene), без `*.pb.h`  
- `tests/` — gtest (свой `main.cpp` + `helpers/`); smoke не в runtime main/engine
