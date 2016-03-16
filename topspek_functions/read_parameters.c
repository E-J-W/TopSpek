//function reads parameter files for the topspek code
void readParFile(const char * fileName, pc_par * par) 
{
  FILE *config;
  char str[256],str1[256],str2[256];
  int index=0;
  par->numSpectra=0;
  par->endSpectrum=0;
  par->maxNumCh=0;
  par->numSimData=0;
  par->numFittedSimData=0;
  memset(par->fixBG,0,sizeof(par->fixBG));
  if((config=fopen(fileName,"r"))==NULL)
    {
      printf("ERROR: Cannot open the config file %s!\n",fileName);
      exit(-1);
    }
  while(!(feof(config)))//go until the end of file is reached
    {
      if(fgets(str,256,config)!=NULL)
        {
        
          if(par->numSimData<NSIMDATA)
            if(sscanf(str,"%i %i %i",&par->spectrum[index],&par->startCh[index],&par->endCh[index])!=3) //no spectrum and channel data
              if(sscanf(str,"%s %s %lf",par->simDataName[par->numSimData],str1,&par->simDataFixedAmpValue[par->numSimData])==3) //simulated dataset info
                {
                  if(strcmp(str1,"yes")==0)
                    par->simDataFixedAmp[par->numSimData]=1;
                  else if(strcmp(str1,"rel")==0)
                    par->simDataFixedAmp[par->numSimData]=2;
                  else
                    {
                      par->simDataFixedAmp[par->numSimData]=0;
                      strcpy(par->fittedSimDataName[par->numFittedSimData],par->simDataName[par->numSimData]);
                      par->numFittedSimData++;
                    }
                  par->numSimData++;
                }
              
          if(index<NSPECT)
            {
              if(sscanf(str,"%i %i %i %s",&par->spectrum[index],&par->startCh[index],&par->endCh[index],str1)==3) //spectrum and channel data
                {
                  if(par->spectrum[index]>par->endSpectrum)
                    par->endSpectrum=par->spectrum[index];
                  if((par->endCh[index]-par->startCh[index]+1)>par->maxNumCh)
                    par->maxNumCh=par->endCh[index]-par->startCh[index]+1;
                  index++;
                  par->numSpectra++;
                }
              if(sscanf(str,"%i %i %i %s %lf %lf %lf",&par->spectrum[index],&par->startCh[index],&par->endCh[index],str1,&par->fixedBGPar[index][0],&par->fixedBGPar[index][1],&par->fixedBGPar[index][2])>=5) //spectrum, channel, and background data
                {
                  if(par->spectrum[index]>par->endSpectrum)
                    par->endSpectrum=par->spectrum[index];
                  if((par->endCh[index]-par->startCh[index]+1)>par->maxNumCh)
                    par->maxNumCh=par->endCh[index]-par->startCh[index]+1;
                  if(strcmp(str1,"yes")==0)
                    par->fixBG[index]=1;
                  index++;
                  par->numSpectra++;
                }
            }
              
          if(sscanf(str,"%s %s",str1,str2)==2) //single parameter data
            {
              if(strcmp(str1,"EXPERIMENT_DATA")==0)
                strcpy(par->expDataName,str2);
              if(strcmp(str1,"ADD_BACKGROUND")==0)
                {
                  if(strcmp(str2,"quad")==0)
                    par->addBackground=2;
                  else if((strcmp(str2,"yes")==0)||(strcmp(str2,"lin")==0))
                    par->addBackground=1;
                  else
                    par->addBackground=0;
                }
              if(strcmp(str1,"PEAK_SEARCH")==0)
                {
                  if(strcmp(str2,"yes")==0)
                    par->peakSearch=1;
                  else
                    par->peakSearch=0;
                }
              if(strcmp(str1,"PLOT_OUTPUT")==0)
                {
                  if(strcmp(str2,"yes")==0)
                    par->plotOutput=1;
                  else if(strcmp(str2,"detailed")==0)
                    par->plotOutput=2;
                  else
                    par->plotOutput=0;
                }
              if(strcmp(str1,"SAVE_OUTPUT")==0)
                {
                  if(strcmp(str2,"yes")==0)
                    par->saveOutput=1;
                  else
                    par->saveOutput=0;
                }
            }
          
          if(sscanf(str,"%s %s",str1,str2)==1) //listing of simulated data
            {
              if(strcmp(str1,"<---END_OF_PARAMETERS--->")==0)
                break;
              else if(strcmp(str1,"SIMULATED_DATA")!=0)
                if(par->numSimData<NSIMDATA)
                  {
                    strcpy(par->simDataName[par->numSimData],str1);
                    par->simDataFixedAmp[par->numSimData]=0;
                    par->simDataFixedAmpValue[par->numSimData]=1.;
                    par->numFittedSimData++;
                    par->numSimData++;
                  }
            }
        }
    }
  fclose(config);
  
  //correct parameters
  for(index=0;index<par->numSpectra;index++)
    if(par->fixBG[index]==0)
      par->fitAddBackground[index]=par->addBackground;
    else if(par->fixBG[index]==1)
      par->fitAddBackground[index]=0;
  
  if(par->addBackground==1)
    for(index=0;index<par->numSpectra;index++)
      par->fixedBGPar[index][2]=0;
  
  //print parameters read from the file
  printf("Taking experiment data from file: %s\n",par->expDataName);
  for(index=0;index<par->numSimData;index++)
    {
      printf("Taking simulated data from file (%i of %i): %s\n",index+1,par->numSimData,par->simDataName[index]);
      if(par->simDataFixedAmp[index]==1)
        printf("Fixing scaling factor for this data to %lf\n",par->simDataFixedAmpValue[index]);
      if(par->simDataFixedAmp[index]==2)
        printf("Fixing scaling factor for this data to a factor of %lf relative to the last fitted data.\n",par->simDataFixedAmpValue[index]);
    }
  if(par->peakSearch==0)
    for(index=0;index<par->numSpectra;index++)
      printf("Will compare spectrum %i from channels %i to %i.\n",par->spectrum[index],par->startCh[index],par->endCh[index]);
  else
    for(index=0;index<par->numSpectra;index++)
      printf("Will search for a peak in spectrum %i from channels %i to %i.\n",par->spectrum[index],par->startCh[index],par->endCh[index]);
  if(par->addBackground==0)
    printf("Will not add background to simulated data.\n");
  if(par->addBackground==1)
    printf("Will add a linear background to simulated data.\n");
  if(par->addBackground==2)
    printf("Will add a quadratic background to simulated data.\n");
  if(par->addBackground==1)
    for(index=0;index<par->numSpectra;index++)
      if(par->fixBG[index]==1)
        printf("Fixing background parameters to A = %lf, B = %lf for spectrum %i, channels %i to %i.\n",par->fixedBGPar[index][0],par->fixedBGPar[index][1],par->spectrum[index],par->startCh[index],par->endCh[index]);
  if(par->addBackground==2)
    for(index=0;index<par->numSpectra;index++)
      if(par->fixBG[index]==1)
        printf("Fixing background parameters to A = %lf, B = %lf, C = %lf for spectrum %i, channels %i to %i.\n",par->fixedBGPar[index][0],par->fixedBGPar[index][1],par->fixedBGPar[index][2],par->spectrum[index],par->startCh[index],par->endCh[index]);
  if(par->plotOutput==0)
    printf("Will not plot output data.\n");
  if(par->plotOutput==1)
    printf("Will plot output data.\n");
  if(par->plotOutput==2)
    printf("Will plot detailed output data.\n");
  if(par->saveOutput==0)
    printf("Will not save fitted simulation data.\n");
  if(par->saveOutput==1)
    {
      if(par->addBackground==0)
        {
          if(par->numSimData>1)
            printf("Will save fitted simulation data to files fit_sim0.mca through fit_sim%i.mca.\n",par->numSimData-1);
          else
            printf("Will save fitted simulation data to file fit_sim0.mca.\n");
        }
      else if(par->addBackground==1)
        {
          if(par->numSimData>1)
            printf("Will save fitted simulation data to files fit_background.mca and fit_sim0.mca through fit_sim%i.mca.\n",par->numSimData-1);
          else
            printf("Will save fitted simulation data to files fit_background.mca and fit_sim0.mca.\n");
        }
    }

  
  printf("Finished reading parameter file...\n");
  
}