ps -aux | grep "\[Genode\] init -> ptcp_test" | awk '{print $2}'
