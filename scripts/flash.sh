if ! type picotool > /dev/null ; then
    echo "picotool not found"
    exit
fi

picotool load -x $1
