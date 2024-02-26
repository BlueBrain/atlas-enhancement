if(!exists("argv")) argv <- commandArgs(trailingOnly = T)
argc <- length(argv)

dat <- read.table(argv[1])
out <- argv[2]
name <- if(argc >= 3) argv[3] else "data"
col <- if(argc >= 4) as.integer(argv[4]) else 1

df <- data.frame(dat[[col]])
colnames(df) <- c(name)

summary(df)
cat(sprintf("Std.dev.:%.4g",sd(df[[name]],na.rm=TRUE)))

pdf(out)
hist(df[[name]],"Scott",xlab = name,main = paste("Histogram of",name))
