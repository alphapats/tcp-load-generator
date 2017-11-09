sudo sysctl net.ipv4.ip_local_port_range="15000 61000"
sudo sysctl net.ipv4.tcp_fin_timeout=10
sudo sysctl net.ipv4.tcp_tw_recycle=1
sudo sysctl net.ipv4.tcp_tw_reuse=1 
sudo sysctl net.core.somaxconn=64000
sudo sysctl net.core.netdev_max_backlog=2000
sudo sysctl net.ipv4.tcp_max_syn_backlog=2048
