#include <maxminddb.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define geodb "data/GeoLite2-City.mmdb"

char location[255];

void geoip_find (const char *ipaddr)
{
    int gai_error, mmdb_error;
    MMDB_s mmdb;
    MMDB_lookup_result_s result;
    MMDB_entry_data_s entry_data;

    memset (location, 0, 255);

    mmdb_error = MMDB_open(geodb, MMDB_MODE_MMAP, &mmdb);
    if (mmdb_error != MMDB_SUCCESS) 
    {
        fprintf(stderr, "\n  Can't open %s - %s\n", geodb, MMDB_strerror(mmdb_error));
        if (mmdb_error == MMDB_IO_ERROR) 
        {
            fprintf(stderr, "    IO error: %s\n", strerror(errno));
        }
    }
    
    result = MMDB_lookup_string(&mmdb, ipaddr, &gai_error, &mmdb_error);

    if (gai_error != 0)
    {
        fprintf (stderr, "Error from getaddrinfo\n");
    }

    if (mmdb_error != MMDB_SUCCESS)
    {   
        fprintf (stderr, "Error from libmaxminddb\n");
    }
   
    if (MMDB_get_value(&result.entry, &entry_data, "country", "iso_code", NULL) != MMDB_SUCCESS)
    {
        fprintf (stderr, "whereis: Error fetching country code for %s\n", ipaddr);
        strcpy (location, "Unknown");
    }
    else
        strncpy (location, entry_data.utf8_string, entry_data.data_size);

    strcat (location, "/");
    if (MMDB_get_value(&result.entry, &entry_data, "city", "names", "en", NULL) != MMDB_SUCCESS)
    {
        fprintf (stderr, "whereis: Error fetching city for %s\n", ipaddr);
        strcat (location, "Unknown");
    }
    else
        strncat (location, entry_data.utf8_string, entry_data.data_size);
    MMDB_close (&mmdb);
    return;
}

