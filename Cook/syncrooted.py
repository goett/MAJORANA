#!/usr/bin/python
# produce 

import sys, getopt
from os import listdir
from os import system
from os.path import isfile, join

def main(argv):
	print 'Number of Arguments:', len(argv) ,'arguments.'
	print 'Argument List:' , str(argv)
	try:
		opts,args = getopt.getopt(argv,"i:o:",["idir=","odir="])
	except getopt.GetoptError:
		print('orca2rootdir.py -i <input directory> -o <output directory>')
		sys.exit(2)
	for opt, arg in opts:
      		if opt in ("-i", "--idir"):
         		inputdir = arg
      		elif opt in ("-o", "--odir"):
         		outputdir = arg
		else:
         		print 'test.py -i <inputdir> -o <outputdir>'
         		sys.exit()
   	print 'Input directory is ', inputdir
   	print 'Output directory is ', outputdir
	#obtain list of files:
	sourcefiles = [ f for f in listdir(inputdir) if isfile(join(inputdir,f)) ]
	outputfiles = [ f for f in listdir(outputdir) if isfile(join(outputdir,f)) ]
	#search for matches:
	for x in sourcefiles:
		match=0
		for y in outputfiles:
			if x in y:
				match=1
		if(match==0):
			#This source file has not been rootified
			file = join(inputdir,x)
			command = 'nice rootify.sh ' + file + ' ' + outputdir
			system(command)
		

if __name__ == "__main__":
   main(sys.argv[1:])


