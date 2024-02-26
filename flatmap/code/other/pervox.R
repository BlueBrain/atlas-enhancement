library(dplyr)
library(ggplot2)

if(!exists("argv")) argv <- commandArgs(trailingOnly = T)
argc <- length(argv)

div <- read.table(argv[1])
lap <- read.table(argv[2])
ort <- read.table(argv[3])

div$metric <- as.factor("divergence")
lap$metric <- as.factor("laplacian")
ort$metric <- as.factor("orthogonality")

dat <- bind_rows(ort,lap,div)

p <- ggplot(dat, aes(V1, metric)) +
        geom_boxplot(aes(colour = metric),
                     outlier.size = 0.5,
                     outlier.alpha = 0.01,
                     outlier.stroke = 0.01) +
        #geom_violin(aes(fill = metric),
        #            scale = 'width',
        #            linewidth = 0) +
        xlab("") + ylab("") +
        theme_classic() +
        theme(axis.line.y = element_blank(),
              axis.ticks.y = element_blank(),
              legend.position = "none")

ggsave(argv[4], p,
    units = "in",
    width = 5,
    height = 2,
    dpi = 300)
