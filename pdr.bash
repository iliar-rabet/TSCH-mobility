tx_num=$( cat $1 | grep "Send" | grep "ID:7" | wc -l)

rx_num=$(cat $1 | grep "Received" | grep "7:7" | wc -l)

result=$(echo "scale=2; $rx_num / $tx_num" | bc)
#result = $($rx / $tx)

echo $result
