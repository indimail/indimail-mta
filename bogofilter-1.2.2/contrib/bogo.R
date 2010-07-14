# bogo.R - check calculations performed by bogofilter
#
# Before running this script,
# prepare file bogo.tbl as follows: run
#   bogofilter -R <mailbox >bogo.tbl
# where mailbox is a file with only one email message in it.

# Then run R and type
#   source("bogo.R")

# first read the data
bogo <- read.table("bogo.tbl")
attach(bogo)

# next read the special values from the last line
l <- length(fw)
bogoS <- fw[l]
s <- invfwlog[l]
x <- fwlog[l]
md <- as.real(levels(U[l])[3])

# now truncate bogo and reset l to match
detach(bogo)
bogo[abs(0.5 - bogo$fw) >= md,] -> bogo
attach(bogo)
l <- length(fw)

# next recalculate the fw values from the counts in the table
attach(bogo)
pw2 <- pbad / (pbad + pgood)
pw2[pbad + pgood == 0] <- 0
fw2 <- (s * x + n * pw2) / (s + n)

# display fw (calculated by bogofilter) and fw2 and compare
print.noquote("R f(w):")
print(round(fw2, digits=6))
print.noquote("")
print.noquote("Bogofilter f(w):")
print(fw)
print.noquote("")
print.noquote("Difference (R - bogo):")
print(round(fw2, digits=6) - fw)

# calculate S using fw2
P <- 1 - exp(sum(log(1-fw2))/l)
Q <- 1 - exp(sum(log(fw2))/l)
Srob <- ( 1 + (P-Q)/(P+Q) ) / 2
S <- pchisq(sum(log(1-fw2)) * -2, 2 * l)
H <- pchisq(sum(log(fw2)) * -2, 2 * l)
Sfis <- (1 + S - H) / 2

Drob <- abs(Srob - bogoS)
Dfis <- abs(Sfis - bogoS)
diff <- min(Drob,Dfis)
RS <- if(Drob < Dfis) Srob else Sfis

# display S as calculated by bogofilter and by R and compare
print.noquote("")
print.noquote(sprintf(
  "R S: %8.2e, Bogofilter S: %8.2e, Difference (R - bogo): %10.4e",
  RS, bogoS, diff))
