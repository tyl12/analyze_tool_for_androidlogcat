#python ./preprocess.py -f logfile -s "BackupManagerService@ActivityManager" -d
#python ./analyze.py -f ./intermediate/_timedifffile -s "@" -d

echo "=========================================================================="
python ./preprocess.py -f logfile -s "callback status : 0@稳定点达到，开始结算"
python ./analyze.py -f ./intermediate/_timedifffile -s "@" -d

echo "=========================================================================="
python ./preprocess.py -f logfile -s "稳定点达到，开始结算@上传中..."
python ./analyze.py -f ./intermediate/_timedifffile -s "@" -d


echo "=========================================================================="
python ./preprocess.py -f logfile -s "上传中...@上传 成功"
python ./analyze.py -f ./intermediate/_timedifffile -s "@" -d
