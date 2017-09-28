DEV_PATH="/dev/"

while read MYDEV
do
	echo "++++++++++++++++"
	echo ${DEV_PATH}${MYDEV}
	echo "ATD10086;\r" >${DEV_PATH}${MYDEV}
done

