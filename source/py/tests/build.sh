# see: https://stackoverflow.com/questions/27672572/embedding-python-in-c-linking-fails-with-undefined-reference-to-py-initialize


echo "compiling..."



for fname in $@
do
    echo "compiling $fname"
    bname=$(basename $fname .c)
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    clang -g -I /usr/include/python3.8 -lpython3.8 $fname -o $bname
	elif [[ "$OSTYPE" == "darwin"* ]]; then
    gcc -g `python3.7-config --cflags --ldflags` $fname -o $bname
	else
		echo "not implemented"
	fi
done

echo "cleaning up..."
rm -rf *.dSYM

