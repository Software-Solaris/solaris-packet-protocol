# tests/

Cgreen unit tests for SPP. Run entirely on a host PC — no ESP32 or hardware required. Uses the `stub` HAL port.

---

## Structure

```
tests/
├── core/
│   └── test_core.c             Tests for SPP_Core_init and port registration
├── services/
│   ├── databank/
│   │   └── test_databank.c     Tests for SPP_Databank_*
│   ├── pubsub/
│   │   └── test_pubsub.c       Tests for SPP_PubSub_*
│   ├── log/
│   │   └── test_log.c          Tests for SPP_Log_*
│   └── test_service.c          Tests for SPP_Service_register / initAll / startAll
└── util/
    └── test_crc.c              Tests for SPP_Util_crc16
```

The test tree mirrors the module tree — every module that has a public API has a corresponding test file under the same relative path.

---

## Running tests

### Via the devcontainer terminal

```bash
run_tests solaris-v1/spp
```

This runs cmake configure + build + ctest in one command.

### Manually

```bash
cd solaris-v1/spp
cmake -S . -B build -DSPP_BUILD_TESTS=ON -DSPP_PORT=posix
cmake --build build
ctest --test-dir build --output-on-failure
```

### With coverage

```bash
cmake -S . -B build -DSPP_BUILD_TESTS=ON -DSPP_PORT=posix \
      -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build
gcov build/CMakeFiles/spp_tests.dir/**/*.gcno
```

---

## Test convention

Each test file focuses on **one module**. Within that file, each Cgreen `Describe` block focuses on **one function**:

```c
// test_pubsub.c

Describe(SPP_PubSub_subscribe);
BeforeEach(SPP_PubSub_subscribe) { SPP_PubSub_init(); }
AfterEach(SPP_PubSub_subscribe)  { /* teardown */ }

Ensure(SPP_PubSub_subscribe, rejects_null_handler) {
    assert_that(SPP_PubSub_subscribe(0x0101U, NULL, NULL),
                is_equal_to(K_SPP_ERROR_NULL_POINTER));
}
```

---

## Adding a new test file

1. Create `tests/<module>/test_<module>.c`
2. Implement `Describe` + `Ensure` blocks
3. Define `TestSuite *<module>_suite(void)` at the bottom
4. Add `spp_add_test_module(spp_test_<module> tests/<module>/test_<module>.c)` to `CMakeLists.txt`
