# Mutex

## Пререквизиты

- [fiber/yield](/tasks/fiber/yield)
- [fiber/sleep_for](/tasks/fiber/sleep_for)

---

Реализуйте примитивы синхронизации для файберов:
- [`Event`](source/exe/fiber/sync/event.hpp)
- [`Mutex`](source/exe/fiber/sync/mutex.hpp)
- [`WaitGroup`](source/exe/fiber/sync/wait_group.hpp)

Методы `Event::Wait`, `Mutex::Lock` и `WaitGroup::Wait` должны останавливать<sup>†</sup> файбер, но не должны блокировать поток планировщика, в котором этот файбер исполняется.

<sup>†</sup> Файберы – _останавливаются_ (_suspend_), потоки – _блокируются_ (_block_).

## Пример

```cpp
void SyncExample() {
  using namespace exe;
  
  runtime::MultiThread rt{4};
  rt.Start();
  
  thread::WaitGroup example;
  example.Add(1);
  
  fiber::Go(rt, [&example] {
    fiber::Mutex mutex;
    size_t cs = 0;
    
    // https://gobyexample.com/waitgroups
    fiber::WaitGroup wg;
    
    for (size_t i = 0; i < 123; ++i) {
      wg.Add(1);
      
      fiber::Go([&] {
        // https://gobyexample.com/defer
        Defer defer([&wg] {
          wg.Done();
        });
        
        for (size_t j = 0; j < 1024; ++j) {
          std::lock_guard guard{mutex};
          ++cs;  // <-- в критической секции
        }
      });
    }
    
    // Дожидаемся завершения группы файберов
    wg.Wait();
    
    fmt::println("# critical sections: {}", cs);
    // <-- Напечатано 123 * 1024
    
    example.Done();
  });
  
  // Дожидаемся завершения примера
  example.Wait();
  
  rt.Stop();
}
```

См. [play/main.cpp](play/main.cpp)

### Кондвар?

Несмотря на то, что в Golang есть [`sync.Cond`](https://pkg.go.dev/sync#Cond), мы намеренно не включаем кондвар
в набор примитивов синхронизации для файберов.

Нужен ли вашим файберам такой примитив?

## Реализация

### SpinLock

Для реализации примитивов синхронизации потребуется взаимное исключение.

Реализуйте и используйте [спинлок](source/exe/thread/spinlock.hpp).

### Fast Path

На _быстром пути_ (_fast path_) в примитивах синхронизации  (`Mutex` свободен, `Event` / `WaitGroup` выполнен) не должно быть переключений контекста.

### Lock-free

Продвинутый уровень!

Напишите лок-фри реализацию `Event`, `Mutex` и `WaitGroup`, без использования взаимного исключения.

### Combining / Symmetric Transfer

Продвинутый уровень!

Реализуйте серийный лок-фри `Mutex`.

Чем больше будет нагружен ваш мьютекс, тем **меньше** будет в исполнении синхронизации (больше локальности по кэшу, меньше атомарных операций) и тем выше будет пропускная способность примитива.

[WIP]

### Дизайн

Возможно вам потребуется доработать API для планирования файберов:

#### Расширяемость

Хороший API позволит (и вам, и пользователю) легко расширять фреймворк новыми примитивами синхронизации и операциями без его доработки.

#### Изоляция

Хороший API позволит изолировать всю синхронизацию в примитивах из `sync`.

Детали реализации (атомики, спинлоки, интрузивные списки) отдельных примитивов синхронизации не должны "протекать" в `core`.

## Задание

1) Реализуйте примитивы синхронизации:
    1) [`Event`](source/exe/fiber/sync/event.hpp)
    2) [`Mutex`](source/exe/fiber/sync/mutex.hpp)
    3) [`WaitGroup`](source/exe/fiber/sync/wait_group.hpp)
2) Оптимизируйте [динамические аллокации](alloc.md)
3) Напишите лок-фри реализацию примитивов синхронизации
4) Напишите серийный лок-фри `Mutex`
5) Оптимизируйте `memory_order` в примитивах синхронизации
