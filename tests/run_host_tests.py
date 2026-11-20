"""Compile and run portable C++ checks plus compile-time configuration rejection cases."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def run() -> None:
    """Build isolated executables; no ESP board, credentials or PlatformIO needed."""
    with tempfile.TemporaryDirectory(prefix="esp-logging-tests-") as tmp:
        common = ["c++", "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pthread",
                  "-I", str(ROOT / "tests/host_stubs"), "-I", str(ROOT / "include")]
        cases = [
            ("test_tcp_core.cpp", []),
            ("test_discovery_protocol.cpp", []),
            ("test_tcp_runtime.cpp", ["-DWIFI_LOGGING_ENABLE=1", "-DWIFI_LOGGING_USE_TCP=1", '-DWIFI_LOGGING_TCP_HOST="192.168.0.249"']),
            ("test_tcp_runtime.cpp", ["-DWIFI_LOGGING_ENABLE=1", "-DWIFI_LOGGING_USE_TCP=1", '-DWIFI_LOGGING_TCP_HOST="192.168.0.249"', "-DTCP_CONNECT_MAX_ATTEMPTS=3"]),
            ("test_tcp_runtime.cpp", ["-DWIFI_LOGGING_ENABLE=1", "-DWIFI_LOGGING_USE_TCP=1", "-DAUTO_SERVER_CONNECT=1"]),
        ]
        for index, (source, flags) in enumerate(cases):
            output = str(Path(tmp) / f"test-{index}")
            subprocess.run(common + flags + [str(ROOT / "tests" / source), "-o", output], check=True)
            subprocess.run([output], check=True, timeout=10)
        source = '#include "log_transport_config.h"\nint main() {}\n'
        config_cases = [([], True), (["-DWIFI_LOGGING_USE_TCP=1", "-DAUTO_SERVER_CONNECT=1"], True),
                        (["-DTCP_CONNECT_MAX_ATTEMPTS=1"], True),
                        (["-DTCP_CONNECT_MAX_ATTEMPTS=0"], False),
                        (["-DTCP_CONNECT_MAX_ATTEMPTS=-1"], False),
                        (["-DTCP_CONNECT_MAX_ATTEMPTS=word"], False),
                        (["-DAUTO_SERVER_CONNECT=2"], False),
                        (["-DAUTO_SERVER_CONNECT=word"], False),
                        (["-DAUTO_SERVER_CONNECT=1"], False),
                        (["-DWIFI_LOGGING_USE_TCP=1"], False),
                        (["-DWIFI_LOGGING_USE_TCP=1", "-DWIFI_LOGGING_USE_UDP=1"], False)]
        for flags, succeeds in config_cases:
            result = subprocess.run(common + flags + ["-x", "c++", "-fsyntax-only", "-"],
                                    input=source, text=True, capture_output=True)
            assert (result.returncode == 0) == succeeds, (flags, result.stderr)
        print(f"Passed {len(cases)} C++ test runs and {len(config_cases)} build-configuration checks")


if __name__ == "__main__":
    run()
