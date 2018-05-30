#COMMAND="./cp -v -c 3 -t 40"

# Default technology

TECHS=lp

if [ "x$@" == "x" ]; then
	echo "Selecting default techonology '$TECHS'"
	sleep 1
else
	TECHS=$@
fi

INDIR="in/"
OUTDIR="out/"
FILES=$(ls -v $INDIR)
#FILES=$(cat hard.$TECH)
TIME_LIMIT=120

for TECH in $TECHS; do

	echo "Using solver $TECH"

	COMMAND="src/$TECH"

	for f in $FILES; do

		FILE_IN=$(basename $f) 
		FILE_OUT=${FILE_IN%.in}_$TECH.out

		# Build complete path
		IN=$INDIR/$FILE_IN
		OUT=$OUTDIR/$FILE_OUT
		LOG=${OUT%.out}.log
		CHK=${OUT%.out}.chk

		echo -n "$FILE_IN... "

		# Test previous work completed
		if [ -e $OUT ]; then
			echo "SKIP"
			continue
		fi

		# Use a temp file (in case of crash or abort)
		timeout $TIME_LIMIT $COMMAND < $IN > $OUT.tmp 2> $LOG.tmp &

		SOLVER_PID=$!

		trap "kill -9 $SOLVER_PID; killall $TECH; exit 1" SIGINT

		wait $!

		if [ "$?" != "0" ]; then
			echo "Time limit or error returned"
			continue
		fi

		# Check that the solution is 'correct'
		src/checker $IN $OUT.tmp > $CHK

		if [ "$?" != "0" ]; then
			echo "The solution is incorrect, saved in $OUT.tmp"
			continue
		fi

		echo "OK"

		# Once completed, move to original

		mv $OUT.tmp $OUT
		mv $LOG.tmp $LOG
	done
done
