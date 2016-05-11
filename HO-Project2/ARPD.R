##
##  ARPD.R
##  HO-Project2
##
##  Created by Jens Nevens on 04/05/16.
##  Copyright © 2016 Jens Nevens. All rights reserved.
##

filepath <- "/Users/Jens/Documents/Jens/School/1MA/Heuristic Optimization/Assignments/Assignment2/HO-Project2/HO-Project2"

## Correlation plot between the percentage deviation of the 2 algorithms
ACO <- read.table(paste(filepath, 'percentage', 'ACO.txt', sep='/'),
						header=FALSE,
						col.names=c('instance', 'percentage'))
GEN <- read.table(paste(filepath, 'percentage', 'GEN.txt', sep='/'),
						header=FALSE,
						col.names=c('instance', 'percentage'))

algorithms <- c('ACO', 'GEN')
instance.sets <- c('4', '5', '6', 'A', 'B', 'C', 'D', 'NRE', 'NRF', 'NRG', 'NRH')
ACO.avg <- c()
GEN.avg <- c()

for (i in instance.sets) {
	ACO.group <- ACO[substr(ACO$instance, 0, nchar(i)) == i,]$percentage
	GEN.group <- GEN[substr(GEN$instance, 0, nchar(i)) == i,]$percentage
	ACO.avg <- c(ACO.avg, mean(ACO.group))
	GEN.avg <- c(GEN.avg, mean(GEN.group))
}


# Calculate range from 0 to max value
g_range <- range(0, ACO.avg, GEN.avg)

pdf(paste(filepath, 'output', 'plots', "ARPD.pdf", sep='/'))
par(mgp=c(2.2,0.45,0), tcl=-0.4, mar=c(3.3,3.6,1.1,1.1))

# Graph autos using y axis that ranges from 0 to max 
# value in cars or trucks vector.  Turn off axes and 
# annotations (axis labels) so we can specify them ourself
plot(ACO.avg, type="o", col="blue", ylim=g_range, axes=FALSE, ann=FALSE)

# Make x axis using Mon-Fri labels
axis(1, at=1:11, lab=instance.sets)

# Make y axis with horizontal labels that display ticks at 
# every 4 marks. 4*0:g_range[2] is equivalent to c(0,4,8,12).
axis(2, tick=0.1)

# Create box around plot
box()

# Graph trucks with red dashed line and square points
lines(GEN.avg, type="o", pch=22, lty=2, col="red")

# Create a title with a red, bold/italic font

# Label the x and y axes with dark green text
title(xlab="Instance Sets")
title(ylab="ARPD (%)")

# Create a legend at (1, g_range[2]) that is slightly smaller 
# (cex) and uses the same line colors and points used by 
# the actual plots 
legend(1, g_range[2], c("ACO","GEN"), cex=0.8, col=c("blue","red"), pch=21:22, lty=1:2)
dev.off()

