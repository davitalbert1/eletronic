#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <type_traits>

namespace test {

namespace detail {

template<typename T, typename = void>
struct is_streamable : std::false_type {};

template<typename T>
struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type {};

template<typename T>
std::string formatValue(const T& val) {
    if constexpr (std::is_enum_v<T>) {
        return std::to_string(static_cast<long long>(val));
    } else if constexpr (is_streamable<T>::value) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    } else {
        return "<unprintable>";
    }
}

} // namespace detail

struct TestFailure {
    std::string file;
    int line = 0;
    std::string expression;
    std::string message;
};

struct TestCaseResult {
    std::string suiteName;
    std::string testName;
    bool passed = true;
    double durationMs = 0.0;
    std::vector<TestFailure> failures;
};

class TestContext {
public:
    static TestContext& instance() {
        static TestContext ctx;
        return ctx;
    }

    void setCurrentTest(const std::string& suite, const std::string& name) {
        currentSuite_ = suite;
        currentTest_ = name;
        currentFailures_.clear();
    }

    void recordFailure(const std::string& file, int line, const std::string& expr, const std::string& msg = "") {
        TestFailure f;
        f.file = file;
        f.line = line;
        f.expression = expr;
        f.message = msg;
        currentFailures_.push_back(f);
    }

    bool hasCurrentFailed() const {
        return !currentFailures_.empty();
    }

    const std::vector<TestFailure>& getCurrentFailures() const {
        return currentFailures_;
    }

private:
    std::string currentSuite_;
    std::string currentTest_;
    std::vector<TestFailure> currentFailures_;
};

using TestFn = std::function<void()>;

struct TestEntry {
    std::string suiteName;
    std::string testName;
    TestFn function;
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry reg;
        return reg;
    }

    void registerTest(const std::string& suite, const std::string& name, TestFn fn) {
        tests_.push_back({suite, name, std::move(fn)});
    }

    const std::vector<TestEntry>& getTests() const {
        return tests_;
    }

    size_t count() const {
        return tests_.size();
    }

private:
    std::vector<TestEntry> tests_;
};

struct TestRegistration {
    TestRegistration(const std::string& suite, const std::string& name, TestFn fn) {
        TestRegistry::instance().registerTest(suite, name, std::move(fn));
    }
};

class TestRunner {
public:
    static int runAll(std::ostream& out = std::cout, const std::string& filter = "") {
        auto& tests = TestRegistry::instance().getTests();
        std::vector<TestCaseResult> results;
        int passedCount = 0;
        int failedCount = 0;

        out << "\n======================================================================\n";
        out << "              EXECUÇÃO DE TESTES AUTOMATIZADOS (C++)\n";
        out << "======================================================================\n";
        if (!filter.empty()) {
            out << "[Filtro ativo]: \"" << filter << "\"\n";
        }
        out << "[Total de testes cadastrados]: " << tests.size() << "\n\n";

        auto totalStart = std::chrono::high_resolution_clock::now();

        std::string currentSuite = "";
        for (const auto& entry : tests) {
            if (!filter.empty()) {
                std::string full = entry.suiteName + "." + entry.testName;
                if (full.find(filter) == std::string::npos &&
                    entry.suiteName.find(filter) == std::string::npos &&
                    entry.testName.find(filter) == std::string::npos) {
                    continue;
                }
            }

            if (entry.suiteName != currentSuite) {
                currentSuite = entry.suiteName;
                out << ">> Suite: " << currentSuite << "\n";
            }

            TestContext::instance().setCurrentTest(entry.suiteName, entry.testName);

            auto start = std::chrono::high_resolution_clock::now();
            try {
                entry.function();
            } catch (const std::exception& ex) {
                TestContext::instance().recordFailure(__FILE__, __LINE__, "std::exception capturada", ex.what());
            } catch (...) {
                TestContext::instance().recordFailure(__FILE__, __LINE__, "Exceção desconhecida capturada");
            }
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(end - start).count();

            TestCaseResult res;
            res.suiteName = entry.suiteName;
            res.testName = entry.testName;
            res.durationMs = duration;
            res.failures = TestContext::instance().getCurrentFailures();
            res.passed = res.failures.empty();

            if (res.passed) {
                passedCount++;
                out << "  [PASS] " << entry.testName 
                    << " (" << std::fixed << std::setprecision(2) << duration << " ms)\n";
            } else {
                failedCount++;
                out << "  [FAIL] " << entry.testName 
                    << " (" << std::fixed << std::setprecision(2) << duration << " ms)\n";
                for (const auto& fail : res.failures) {
                    out << "         -> ERRO em " << fail.file << ":" << fail.line << "\n";
                    out << "            Expressão: " << fail.expression << "\n";
                    if (!fail.message.empty()) {
                        out << "            Detalhe  : " << fail.message << "\n";
                    }
                }
            }
            results.push_back(res);
        }

        auto totalEnd = std::chrono::high_resolution_clock::now();
        double totalDuration = std::chrono::duration<double, std::milli>(totalEnd - totalStart).count();

        out << "\n----------------------------------------------------------------------\n";
        out << "RELATÓRIO FINAL DOS TESTES:\n";
        out << "  Passaram: " << passedCount << "\n";
        out << "  Falharam: " << failedCount << "\n";
        out << "  Total executado: " << (passedCount + failedCount) << "\n";
        out << "  Tempo total: " << std::fixed << std::setprecision(2) << totalDuration << " ms\n";

        if (failedCount == 0) {
            out << "\n>>> SUCESSO: Todos os testes passaram sem erros! <<<\n";
            out << "======================================================================\n\n";
            return 0;
        } else {
            out << "\n>>> FALHA: Foram encontrados " << failedCount << " teste(s) com erro(s)! <<<\n";
            out << "Lista de falhas apontadas:\n";
            for (const auto& res : results) {
                if (!res.passed) {
                    out << "  * [" << res.suiteName << "::" << res.testName << "]\n";
                    for (const auto& f : res.failures) {
                        out << "      " << f.file << ":" << f.line << " -> " << f.expression;
                        if (!f.message.empty()) out << " (" << f.message << ")";
                        out << "\n";
                    }
                }
            }
            out << "======================================================================\n\n";
            return 1;
        }
    }
};

} // namespace test

// Macros de Asserção
#define TEST_CONCAT_INNER(a, b) a##b
#define TEST_CONCAT(a, b) TEST_CONCAT_INNER(a, b)

#define TEST_CASE(suite, name) \
    void TEST_CONCAT(test_##suite##_, name)(); \
    static ::test::TestRegistration TEST_CONCAT(reg_##suite##_, name)(#suite, #name, TEST_CONCAT(test_##suite##_, name)); \
    void TEST_CONCAT(test_##suite##_, name)()

#define TEST_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            ::test::TestContext::instance().recordFailure(__FILE__, __LINE__, #cond); \
        } \
    } while (0)

#define TEST_ASSERT_MSG(cond, msg) \
    do { \
        if (!(cond)) { \
            std::ostringstream _oss_msg; \
            _oss_msg << msg; \
            ::test::TestContext::instance().recordFailure(__FILE__, __LINE__, #cond, _oss_msg.str()); \
        } \
    } while (0)

#define TEST_ASSERT_EQ(a, b) \
    do { \
        auto _valA = (a); \
        auto _valB = (b); \
        if (!(_valA == _valB)) { \
            std::ostringstream _oss_eq; \
            _oss_eq << "Esperado: " << ::test::detail::formatValue(_valB) << " | Obtido: " << ::test::detail::formatValue(_valA); \
            ::test::TestContext::instance().recordFailure(__FILE__, __LINE__, #a " == " #b, _oss_eq.str()); \
        } \
    } while (0)

#define TEST_ASSERT_NE(a, b) \
    do { \
        auto _valA = (a); \
        auto _valB = (b); \
        if (_valA == _valB) { \
            std::ostringstream _oss_ne; \
            _oss_ne << "Valores não deveriam ser iguais: " << ::test::detail::formatValue(_valA); \
            ::test::TestContext::instance().recordFailure(__FILE__, __LINE__, #a " != " #b, _oss_ne.str()); \
        } \
    } while (0)

#define TEST_ASSERT_TRUE(cond) TEST_ASSERT(cond)
#define TEST_ASSERT_FALSE(cond) TEST_ASSERT(!(cond))

#define TEST_ASSERT_NEAR(a, b, eps) \
    do { \
        double _diff = std::abs(static_cast<double>(a) - static_cast<double>(b)); \
        if (_diff > (eps)) { \
            std::ostringstream _oss_near; \
            _oss_near << "Esperado: " << (b) << " | Obtido: " << (a) << " | Diferença: " << _diff << " > " << (eps); \
            ::test::TestContext::instance().recordFailure(__FILE__, __LINE__, "|" #a " - " #b "| <= " #eps, _oss_near.str()); \
        } \
    } while (0)
