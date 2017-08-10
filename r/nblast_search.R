
args = commandArgs(trailingOnly=TRUE)

if (length(args) < 3) stop("At least two argument must be supplied.", call.=FALSE)
imagefile = args[1]
nlibpath  = args[2]

if (length(args) >= 3) {
  outfname = args[3]
} else {
  outfname = paste(basename(imagefile), ".nblust", sep="")
}

if (length(args) >= 4) {
	outputdir = args[4]
} else {
	outputdir = dirname(imagefile)
}
if (!dir.exists(outputdir)) {
    dir.create(outputdir, FALSE)
}

if (length(args) >= 5) {
	resultnum = strtoi(args[5])
	if (is.na(resultnum)) resultnum = 10
} else {
  resultnum = 10
}

cat("Loading NAT...")
library(nat.nblast)
library(nat)

cat("Loading neuron libraries...\n")
dp = read.neurons(nlibpath, pattern = 'rda$')

cat("Loading images...\n")
img = read.im3d(imagefile)

cat("Running NBLAST...\n")
scores = nblast(dotprops(img), dp, normalised=T)
scores = sort(scores, dec=T)

if (length(scores) <= resultnum) {
  results = dp[names(scores)]
  slist = scores
} else {
  results = dp[names(scores)[1:resultnum]]
  slist = scores[1:resultnum]
}

cat("Writing results...\n")
swczipname = paste(outfname, ".zip", sep="")
rlistname  = paste(outfname, ".txt", sep="")

write.neurons(results, dir=file.path(outputdir,swczipname), files=names(results), format='swc', Force=T)
write.table(format(slist, digits=15), file.path(outputdir,rlistname), sep=",", quote=F, col.names=F, row.names=T)

cat("Done\n")
