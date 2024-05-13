library(dplyr)
library(ggplot2)

if(!exists("argv")) argv <- commandArgs(trailingOnly = T)
argc <- length(argv)

div_txt = argv[1]
lap_txt = argv[2]
ort_txt = argv[3]
out_png = argv[4]

div <- read.table(div_txt, na.strings = c("nan"))
lap <- read.table(lap_txt, na.strings = c("nan"))
ort <- read.table(ort_txt, na.strings = c("nan"))

div$metric <- as.factor("divergence")
lap$metric <- as.factor("laplacian")
ort$metric <- as.factor("orthogonality")

dat <- bind_rows(ort,lap,div)

p <- ggplot(dat, aes(V1, metric)) +
     geom_boxplot(aes(colour = metric),
                  outlier.size = 0.5,
                  outlier.alpha = 0.01,
                  outlier.stroke = 0.01) +
     xlab("") +
     ylab("") +
     theme_classic() +
     theme(axis.line.y = element_blank(),
           axis.ticks.y = element_blank(),
           legend.position = "none")

ggsave(out_png, p,
       units = "in",
       width = 5,
       height = 2,
       dpi = 300)
