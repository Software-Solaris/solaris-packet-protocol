# tests/

Cgreen unit tests for SPP. Run entirely on a host PC — no ESP32 or hardware required. Uses the `posix` OSAL port and the `stub` HAL.

---

## Structure

```
tests/
├── run_all_tests.c         Entry point — aggregates all suites
├── helpers.h               Shared test utilities and fixtures
├── core/
│   └── test_core.c         Tests for SPP_Core_init and port registration
├── services/
│   ├── databank/
│   │   └── test_databank.c Tests for SPP_Databank_*
│   ├── db_flow/
│   │   └── test_db_flow.c  Tests for SPP_DbFlow_*
│   ├── log/
│   │   └── test_log.c      Tests for SPP_Log_*
│   └── test_service.c      Tests for SPP_Service_register / startAll
└── util/
    └── test_crc.c          Tests for SPP_Util_crc16
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
// test_databank.c

Describe(SPP_Databank_init);
BeforeEach(SPP_Databank_init) { /* reset state */ }
AfterEach(SPP_Databank_init)  { /* teardown    */ }

Ensure(SPP_Databank_init, returns_ok_on_first_call) {
    assert_that(SPP_Databank_init(), is_equal_to(K_SPP_OK));
}

Ensure(SPP_Databank_init, returns_already_initialized_on_second_call) {
    SPP_Databank_init();
    assert_that(SPP_Databank_init(), is_equal_to(K_SPP_ERROR_ALREADY_INITIALIZED));
}
```

Declare the suite function at the bottom of the file, and add it to `run_all_tests.c`.

---

## Adding a new test file

1. Create `tests/<module>/test_<function>.c`
2. Implement `Describe` + `Ensure` blocks
3. Define `TestSuite *<module>_<function>_suite(void)` at the bottom
4. Add the declaration and `add_suite()` call to `run_all_tests.c`
5. The `CMakeLists.txt` picks it up automatically via `file(GLOB_RECURSE ...)`
