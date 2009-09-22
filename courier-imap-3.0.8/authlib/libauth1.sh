
# $Id: libauth1.sh,v 1.1 2000/10/05 04:15:41 mrsam Exp $

while read L
do
   REVLIST="$L $REVLIST"
done
echo $REVLIST | tr ' ' '\012'
