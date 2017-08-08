
args = commandArgs(trailingOnly=TRUE)

if (length(args) < 2) stop("At least two argument must be supplied.", call.=FALSE)
imagefile = commandArgs(trailingOnly=TRUE)[1]
nlibpath  = commandArgs(trailingOnly=TRUE)[2]

if (length(args) >= 3) {
	outputdir = commandArgs(trailingOnly=TRUE)[3]
} else {
	outputdir = paste(imagefile, ".nblast", sep="")	
}
if (!dir.exists(outputdir)) {
    dir.create(outputdir, FALSE)
}

if (length(args) >= 4) {
	resultnum = strtoi(commandArgs(trailingOnly=TRUE)[4])
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
open3d()

cat("Writing results...\n")
for (i in results) {
	plot3d(i)
  ofname = paste(names(i), ".obj", sep = "")
	writeOBJ(file.path(outputdir, ofname))
	cat(paste(opath, "\n", sep = ""))
	clear3d()
}

write.table(slist, file.path(outputdir, "results.txt"), sep=",", quote=F, col.names=F, row.names=T)

cat("Done\n")
