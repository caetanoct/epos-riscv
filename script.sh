PID=`lsof -i :1235 | grep v4 | awk -F ' ' '{print $2}'`
echo "killed $PID"
kill -9 $PID
