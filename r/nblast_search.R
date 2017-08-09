
args = commandArgs(trailingOnly=TRUE)

if (length(args) < 2) stop("At least two argument must be supplied.", call.=FALSE)
imagefile = args[1]
nlibpath  = args[2]

if (length(args) >= 3) {
	outputdir = args[3]
} else {
	outputdir = dirname(imagefile)
}
if (!dir.exists(outputdir)) {
    dir.create(outputdir, FALSE)
}

if (length(args) >= 4) {
	resultnum = strtoi(args[4])
	if (is.na(resultnum)) resultnum = 10
} else {
  resultnum = 10
}

cat("Loading NAT...")
library(nat)
library(nat.nblast)

cat("Loading neuron libraries...\n")
dp = read.neurons(nlibpath, pattern = 'rda$')

cat("Loading images...\n")
img = read.im3d(imagefile)

cat("Running NBLAST...\n")
scores = nblast(img, dp, normalised=T)
scores = sort(scores, dec=T)
sorted = dp[names(scores)]

if (length(sorted) <= resultnum) {
  results = sorted
  slist = scores
} else {
  results = sorted[1:resultnum]
  slist = scores[1:resultnum]
}

cat("Writing results...\n")
swczipname = paste(basename(imagefile), ".nblust.zip", sep="")
rlistname  = paste(basename(imagefile), ".nblust.txt", sep="")

write.neurons(results, dir=swczipname, files=names(results), format='swc')
write.table(slist, file.path(outputdir, rlistname), sep=",", quote=F, col.names=F, row.names=T)

cat("Done\n")
