
args = commandArgs(trailingOnly=TRUE)

if (length(args) < 3) stop("At least two argument must be supplied.", call.=FALSE)
imagefile = args[1]

nlibpath  = args[2]
nlibs = strsplit(nlibpath, ",")
nlibs = nlibs[[1]]

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

if (length(args) >= 6) {
  dbnames = args[6]
} else {
  dbnames = paste(basename(nlibs), collapse=",")
}

cat("Loading NAT...")

if (!require("nat",character.only = TRUE)) {
  if (!requireNamespace("devtools")) install.packages("devtools")
  devtools::install_github("jefferis/nat")
}
if (!require("nat.nblast",character.only = TRUE)) {
  devtools::install_github("jefferislab/nat.nblast")
}
#if (!require("foreach",character.only = TRUE)) {
#  install.packages("foreach", repos="http://cran.rstudio.com/")
#}
#if (!require("doParallel",character.only = TRUE)) {
#  install.packages("doParallel", repos="http://cran.rstudio.com/")
#}

library(nat.nblast)
library(nat)
#library(foreach)
#library(parallel)
#library(doParallel)

cat("Loading images...\n")
img = read.im3d(imagefile)

cat("Running NBLAST...\n")

#cl <- parallel::makeCluster(parallel::detectCores()-1)
#registerDoParallel(cl)

allres = neuronlist();
allscr = numeric();

for (i in 1:length(nlibs)) {
  cat(paste(nlibs[i], "\n"))
  dp = read.neuronlistfh(nlibs[i], localdir=dirname(nlibs[i]))

  #scores = nblast(dotprops(img), dp, normalised=T, UseAlpha=T, .parallel=T, .progress='text')
  scores = nblast(dotprops(img), dp, normalised=T, UseAlpha=T, .progress='text')
  scores = sort(scores, dec=T)
  
  #scores = nblast(dotprops(img), dp, normalised=F, version=1, sd=3, .progress='text')
  #scores = sort(scores, dec=F)

  if (length(scores) <= resultnum) {
    results = as.neuronlist(dp[names(scores)])
    slist = scores
  } else {
    results = as.neuronlist(dp[names(scores)[1:resultnum]])
    slist = scores[1:resultnum]
  }
  
  for (j in 1:length(results)) {
    names(results)[j] = paste(names(results[j]), as.character(i-1), sep=",")
  }
  for (j in 1:length(slist)) {
    names(slist)[j] = paste(names(slist[j]), as.character(i-1), sep=",")
  }
  
  allres = c(allres, results)
  allscr = c(allscr, slist)
  
  rm(dp)
  gc()
}

allscr = sort(allscr, dec=T)
if (length(allscr) <= resultnum) {
  results = allres[names(allscr)]
  slist = allscr
} else {
  results = allres[names(allscr)[1:resultnum]]
  slist = allscr[1:resultnum]
}

cat("Writing results...\n")
swczipname = paste(outfname, ".zip", sep="")
rlistname  = paste(outfname, ".txt", sep="")
zprojname  = paste(outfname, ".png", sep="")

f = file(file.path(outputdir,rlistname))
writeLines(c(dbnames, nlibpath), con=f)
write.table(format(slist, digits=8), append=T, file.path(outputdir,rlistname), sep=",", quote=F, col.names=F, row.names=T)
#n = names(results)
#n = gsub(",", " db=", n)
#write.neurons(results, dir=file.path(outputdir,swczipname), files=n, format='swc', Force=T)

zproj = projection(img, projfun=max)
size = dim(zproj)
png(file.path(outputdir,zprojname), size[1], size[2])
par(plt=c(0,1,0,1))
image(zproj, col = grey(seq(0, 1, length = 256)))
dev.off()

#stopCluster(cl)

cat("Done\n")
