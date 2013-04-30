/*
  convert Synopsys TetraMAX STIL format test pattern file (.pattern) to .atp file
  by zhaoyang, 2007/01/25
  
  input: .pattern (Synopsys's STIL format test pattern file)
  output: .atp (a test pattern pair contains two rows, could be used for dimitri or pomeranz's path delay fault simulation)
  command: pattern2atp s27fs

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main3(int argc, char *argv[])
{
	FILE *fpr, *fpw;
	int i = 0,j = 0;
	int bit_s, bit_vector, num_pattern;
	int start_flag = 0;
	char filename[100], s[10000], launch_vector[10000], capture_vector[10000];
	char tmp_s[10000];
	char *position;

	if (argc == 1)
	{
		printf("input the circuit data file name like this\n");
		printf("scan datafile name\n");
		return -2;
	}

	strcpy(filename, argv[1]);
	strcat(filename, ".pattern");
	if ((fpr = fopen(filename, "r")) == NULL)
	{
		printf("Can not open file %s,", filename);
		return -3;
	}

	strcpy(filename, argv[1]);
	strcat(filename, ".syn.atp");
	if ((fpw = fopen(filename, "w+")) == NULL)
	{
		printf("Can not open file %s,", filename);
		return -3;
	}  

	num_pattern = 0;
	fgets(s, 10000, fpr);
	while (!feof(fpr))
	{
		if (strstr(s, "Ann {* full_sequential *}") != NULL)
		{
			num_pattern++;

			//'lauch vector of currrent pattern
			strcpy(s, "\0");
			do
			{
				fgets(tmp_s, 10000, fpr);
				if ((position = strchr(tmp_s, '\n')) != NULL)
				{
					*position = '\0';
				}
				strcat(s, tmp_s);
			}
			while (strstr(tmp_s, ";") == NULL);
			//printf("%s", s);

			bit_s = 0;
			while (s[bit_s] != '=')
			{
				bit_s++;
			}
			bit_vector = 0;
			for (i = bit_s + 1; s[i] != ';'; i++)
			{
				launch_vector[bit_vector++] = s[i];
			}
			launch_vector[bit_vector] = '\0';

			//capture vector of current pattern
			strcpy(s, "\0");
			do
			{
				fgets(tmp_s, 10000, fpr);
				if ((position = strchr(tmp_s, '\n')) != NULL)
				{
					*position = '\0';
				}
				strcat(s, tmp_s);
			}
			while (strstr(tmp_s, ";") == NULL);

			bit_s = 0;
			while (s[bit_s] != '=')
			{
				bit_s++;
			}
			bit_vector = 0;
			for (i = bit_s + 1; s[i] != ';'; i++)
			{
				capture_vector[bit_vector++] = s[i];
			}
			capture_vector[bit_vector] = '\0';

			//write to .syn.atp
			if (start_flag == 0)
			{
				//		fprintf(fpw, "%d\n", bit_vector);
				start_flag = 1;
			}
			fprintf(fpw, "%s\n", launch_vector);
			//		fprintf(fpw, "%s\n", capture_vector);
		}

		//other lines
		fgets(s, 10000, fpr);
	}

	fprintf(fpw, "END");
	fclose(fpr);
	fclose(fpw);

	printf("\n  %d test patterns are converted.\n", num_pattern);
	return 0;
}
