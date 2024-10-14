lib\qd_test.exe -v > log.txt
lib\pslq_test.exe -v >> log.txt
lib\qd_timer.exe -v >> log.txt
lib\quadt_test.exe -v >> log.txt
type log.txt

lib\qd_test_static.exe -v > log_static.txt
lib\pslq_test_static.exe -v >> log_static.txt
lib\qd_timer_static.exe -v >> log_static.txt
lib\quadt_test_static.exe -v >> log_static.txt
type log_static.txt
