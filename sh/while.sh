
i=0

while [ ${i} -le 100 ]
do
	echo ${i}
	let "i++"
	ps | grep com.android.phone
done

