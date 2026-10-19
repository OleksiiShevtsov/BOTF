# Добавление новой нотификации

Как провести новое сообщение через wire без поломки слоёв.  
Краткий checklist — в [README](../README.md#новая-нотификация-checklist).

## Поток (всегда один)

```
app / engine
  → protocol::encodeX(DTO, sequence)     // Envelope oneof body
  → net::OutboundNotification{ payload }
  → TCP [uint32 le length][Envelope bytes]
  → inbound queue (сырой payload)
  → protocol::decode → DecodedEnvelope{ MessageBody, sequence }
  → protocol::dispatch(visitor) → handleX / apply
  → GameLogic / Match (без *.pb.h)
```

`net/` **не знает** тип сообщения — только байты и framing.  
Смысл живёт в `protocol/` + DTO в `common/`.

## Checklist

1. **Proto** — новый `.proto` при необходимости; поле в `Envelope.body` oneof (`common/protobuf/messages/envelope.proto`).
2. **DTO** — структура в `common/sources/include/common/dto.hpp` (чем думает логика).
3. **Mappers** — `toProto` / `fromProto` в `protocol/include/protocol/mappers.hpp`.
4. **MessageBody** — добавить DTO в `std::variant` в `protocol/include/protocol/messages.hpp`.
5. **Codec** — `encodeX(...)` в `protocol/include/protocol/codec.hpp`  
   (`decode` читает `envelope.body_case()` — добавить ветку `kYourMessage`).
6. **Отправитель** — `OutboundNotification{ clientId?, encodeX(...) }`  
   (клиент: `ClientApp`; сервер: `ServerEngine`).
7. **Получатель** — ветка в `protocol::dispatch` visitor  
   (`ServerEngine::applyInbound` и/или `ClientApp::applyInbound`) → handler → logic/UI.
8. **Net не трогать** — framing/очереди уже общие.
9. **Тест (желательно)** — encode/decode round-trip или smoke через engine.

После смены `.proto` — пересобрать target `proto_messages` / полный `tests`.

## Кто кого шлёт

| Направление | Примеры сейчас | encode | handle |
|-------------|----------------|--------|--------|
| Client → server | `Hello`, `ClientInput` | `ClientApp` | `ServerEngine` → `GameLogic` |
| Server → client | `Hello` ack, `WorldState` | `ServerEngine` | `ClientApp` → Match/Scene |

Новый gameplay-ввод чаще расширяет **`ClientInput`**, а не плодит отдельный msg.  
Новый кусок мира чаще расширяет **`WorldState`** / DTO снапшота.

## Когда не создавать отдельное сообщение

- Ещё одна кнопка/флаг ввода на тик → поле в `ClientInput` (как `fire`).
- Ещё один вид сущности в кадре мира → поле/repeated в `WorldState` + view на клиенте (как `bullets`).
- Отдельный msg — для редких session/control событий (`Hello`) или если пакет принципиально другой по частоте/адресату.

## Константы

Gameplay/runtime числа (cooldown, speed, sizes) → `botf::common::GlobalSettings`  
с комментарием `// TODO: load from DB later`.  
Не хардкодить магические числа в `draw` / `tryFire` без настроек.

## Пример минимального diff (ориентир)

Новое server→client событие `Foo`:

1. `foo.proto` + `Foo foo = N;` в `Envelope.body`
2. `struct FooMessage { ... };` в `dto.hpp`
3. mappers + `MessageBody` + `encodeFoo` + `decode` case `kFoo`
4. `ServerEngine` где нужно: `encodeFoo` → outbound  
5. `ClientApp::applyInbound` visitor: `if constexpr (FooMessage)` → apply  
6. logic/UI без protobuf includes

## Связанные файлы

| Слой | Путь |
|------|------|
| Proto | `common/protobuf/messages/` |
| DTO | `common/sources/include/common/dto.hpp` |
| Codec / dispatch | `protocol/include/protocol/{codec,dispatch,messages,mappers}.hpp` |
| Net | `net/` (не менять ради типа msg) |
| Server apply | `server/engine/src/server_engine.cpp` |
| Client apply | `client/app/src/client_app.cpp` |
| Style / слои | [`STYLE.md`](STYLE.md) |
