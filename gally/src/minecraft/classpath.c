#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../utils.h"
#include "../cjson/cJSON.h"

int compareLwjglVersion(char* new, char* old)
{
	for (int i = 0; i < (int)strlen(new); i++)
	{
		if (new[i] > old[i])
			return 0;
		else if (new[i] < old[i])
			return 1;
	}
	return 1;
}

char* str_split(char* text, char chr, int index)
{
    int i = 0;
    size_t len_element = 0;
    char* element = NULL;

    while (index>0 && text[i] != '\0')
    {
        while (text[i] != chr && text[i] != '\0')
            i++;
        index--;
        i++;
    }
    
    if (text[i] == '\0')
        return NULL;

    while (text[i+len_element] != chr && text[i+len_element] != '\0')
        len_element++;


    element = malloc(sizeof(char) * (len_element+1));
    for (size_t j = 0; j < len_element; j++)
        element[j] = text[i+j];
    element[len_element] = '\0';
    return element;
}

char* mc_GetLwjglVersion(cJSON* manifest)
{
	char* name = NULL;
	char* version = NULL;

	char* lwjglVersion = malloc(sizeof(char) * 6);
    strcpy(lwjglVersion, "0.0.0");

    cJSON* libName = NULL;
    cJSON* lib = NULL;

    cJSON* libraries = cJSON_GetObjectItemCaseSensitive(manifest, "libraries");
	if (libraries)
	{
		cJSON_ArrayForEach(lib, libraries)
		{
            
            libName = cJSON_GetObjectItemCaseSensitive(lib, "name");
            if (libName)
            {
                name = str_split(libName->valuestring, ':', 1);
                if (strcmp(name, "lwjgl") == 0)
                {
                    version = str_split(libName->valuestring, ':', 2);
                    if (compareLwjglVersion(version, lwjglVersion) == 0)
                    {
                        free(lwjglVersion);
                        lwjglVersion = version;
                    } 
                    else
                        free(version);
                }
                free(name);
            }
        }
    }
    

	return lwjglVersion;
}

char* mc_DownloadLibraries(cJSON *manifest, char *path)
{
    int isCorrectOs = 0;

    size_t len_path = strlen(path);
	char* fullpath = NULL;
	char* org = NULL;
	char* name = NULL;
	char* version = NULL;
    char* native = NULL;

    size_t len_native = 0;
    size_t len_name = 0;
    size_t len_version = 0;
    size_t len_org = 0;

	int isLwjgl = 0;

	char* classpath = malloc(sizeof(char));
	strcpy(classpath, "");

	cJSON* lib = NULL;
	cJSON* libraries = cJSON_GetObjectItemCaseSensitive(manifest, "libraries");
    cJSON* tmp = NULL;
    cJSON* tmp_i = NULL;
    cJSON* libDlInfo = NULL;

	char* lwjglClasspath = "";
	char* lwjglVersion = "0.0.0";

    char* libNameFormatted = NULL;
    char* libUrl = NULL;
    cJSON* libName = NULL;
    cJSON* urlInfo = NULL;


	if (libraries)
	{
		cJSON_ArrayForEach(lib, libraries)
		{
            isCorrectOs = 1;
	        tmp = cJSON_GetObjectItemCaseSensitive(lib, "rules");
            if (tmp)
            {
                cJSON_ArrayForEach(tmp_i, tmp)
                {
                    tmp = cJSON_GetObjectItemCaseSensitive(tmp_i, "os");
                    if (tmp)
                    {
                        tmp = cJSON_GetObjectItemCaseSensitive(tmp, "name");
                        if (tmp)
                        {
                            if (strcmp(OSNAME, tmp->valuestring) != 0)
                            {
                                isCorrectOs = 0;
                            }
                        }
                    }
                }
            }
            if (isCorrectOs == 0)
                continue;

			libName = cJSON_GetObjectItemCaseSensitive(lib, "name");
            size_t len_libName = strlen(libName->valuestring);
			libNameFormatted = NULL;


			fullpath = NULL;
			isLwjgl = 0;

            org = str_split(libName->valuestring, ':', 0);
            len_org = strlen(org);

            name = str_split(libName->valuestring, ':', 1);
            len_name = strlen(name);

            version = str_split(libName->valuestring, ':', 2);
            len_version = strlen(version);

            native = str_split(libName->valuestring, ':', 3);
            if (native != NULL)
                len_native = strlen(native);
            else
                len_native = 0;
            
			size_t len_libNameFormatted = len_libName + len_name + len_version + len_native + 7;
			libNameFormatted = realloc(libNameFormatted, sizeof(char) * len_libNameFormatted);

            for (int i=0; org[i] != '\0'; i++)
            {
                if (org[i] == '.')
                    org[i] = '/';
            }

			snprintf(libNameFormatted, len_libNameFormatted, "%s/%s/%s/%s-%s", org, name, version, name, version);
            if (len_native != 0)
            {
                strncat(libNameFormatted, "-", len_libNameFormatted);
                strncat(libNameFormatted, native, len_libNameFormatted);
                free(native);
            }

            strncat(libNameFormatted, ".jar", len_libNameFormatted);

			size_t len_fullpath = (len_path + len_libNameFormatted + 2);
			fullpath = realloc(fullpath, sizeof(char) * len_fullpath);
			snprintf(fullpath, len_fullpath, "%s/%s", path, libNameFormatted);
			
			if (isLwjgl)
			{
				if (len_version >= 5)
					version[5] = '\0';

				if (strcmp(lwjglVersion, version) != 0)
				{
					if (compareLwjglVersion(version, lwjglVersion) == 0)
					{
						lwjglClasspath = realloc(lwjglClasspath, sizeof(char) * (strlen(fullpath) + 1));
						strcpy(lwjglClasspath, fullpath);
						strcpy(lwjglVersion, version);
					}
					else
						continue;
				}
				lwjglClasspath = realloc(lwjglClasspath, sizeof(char) * (strlen(lwjglClasspath) + strlen(fullpath) + 1));
				strcat(lwjglClasspath, CLASSSEPARATOR);
				strcat(lwjglClasspath, fullpath);
			}
			else
			{
				classpath = realloc(classpath, sizeof(char *) * (strlen(classpath) + strlen(fullpath) + 1));
				strcat(classpath, fullpath);
				strcat(classpath, CLASSSEPARATOR);
			}
				// Download Librarie
			libDlInfo = cJSON_GetObjectItemCaseSensitive(lib, "downloads");
			if (libDlInfo)
			{	
				libDlInfo = cJSON_GetObjectItemCaseSensitive(libDlInfo, "artifact");
				if (libDlInfo)
				{
					urlInfo = cJSON_GetObjectItemCaseSensitive(libDlInfo, "url");
					http_Download(urlInfo->valuestring, fullpath);
				}
			} else
			{
				libDlInfo = cJSON_GetObjectItemCaseSensitive(lib, "url");
				if (libDlInfo)
				{
					size_t len_libUrl = strlen(libDlInfo->valuestring) + strlen(libNameFormatted) + 1;
					libUrl = realloc(libUrl, sizeof(char) * len_libUrl);
					snprintf(libUrl, len_libUrl, "%s%s", libDlInfo->valuestring, libNameFormatted);
					http_Download(libUrl, fullpath);	
				}
			}

            free(fullpath);
            free(libNameFormatted);
            free(org);
            free(name);
            free(version);
		}
	}

	if (strcmp(lwjglClasspath, "") != 0)
	{
		classpath = realloc(classpath, strlen(classpath) + strlen(lwjglClasspath) + 1);
		strcat(classpath, lwjglClasspath);
	}


	return classpath;
}
