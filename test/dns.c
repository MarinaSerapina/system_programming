
/*************************************************************************
   LAB 1

	Edit this file ONLY!

*************************************************************************/



#include "dns.h"
#include <stdio.h>
#pragma warning(disable: 4996)
#define CAPACITY 15000
#define FALSE 0
#define TRUE  1

typedef struct node
{
	char name[100];
	int id;
	struct node* next;
}
node;

unsigned int hash(char* str) {
	unsigned int hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c;

	return hash % 15000;
}




void ht_insert(node** ht, const char* name, int id)
{
	int key = hash(name);
	node* newptr = malloc(sizeof(node));
	if (newptr == NULL)
	{
		return;
	}
	strcpy(newptr->name, name);
	newptr->id = id;
	newptr->next = NULL;

	if (ht[key] == NULL)
	{
		ht[key] = newptr;
	}
	else
	{
		node* predptr = ht[key];
		while (TRUE)
		{
			if (predptr->next == NULL)
			{
				predptr->next = newptr;
				break;
			}

			predptr = predptr->next;
		}
	}
}

int ht_search(node** ht, char* name) {
	int key = hash(name);
	node* ptr = ht[key];

	while (ptr != NULL) {
		if (strcmp(ptr->name, name) == 0)
			return ptr->id;
		ptr = ptr->next;
	}
	return -1;
}



DNSHandle InitDNS()
{
	node** ht = malloc(sizeof(node*) * CAPACITY);
	for (int i = 0; i < CAPACITY; i++)
		ht[i] = NULL;
	


	return ht;
}

void LoadHostsFile(DNSHandle hDNS, const char* hostsFilePath)
{
	
	FILE* hostsFile;
	char str[256];
	char domainName[256];
	int* n1 = 0,* n2 = 0,* n3 = 0, *n4 = 0;

	IPADDRESS ip = 0;

	hostsFile = fopen(hostsFilePath, "r");

	while (fgets(str, 255, hostsFile)) {		
		sscanf(str, "%d.%d.%d.%d %s",  &n1, &n2, &n3, &n4, domainName);
		ip = CreateIP(n1, n2, n3, n4);
		ht_insert((node**)hDNS,domainName, ip);
	}
	fclose(hostsFile);
}

IPADDRESS CreateIP(unsigned char _first, unsigned char _second, unsigned char _third, unsigned char _fourth) {

	IPADDRESS ip = (IPADDRESS)(_first << 24) + (_second << 16) + (_third << 8) + _fourth;

	return ip;
}

IPADDRESS DnsLookUp(DNSHandle hDNS, const char* hostName)
{
	int ip = ht_search((node**)hDNS,hostName);

	if(ip==-1)
		return INVALID_IP_ADDRESS;
	return ip;
}

void ShutdownDNS(DNSHandle hDNS)
{
	node** t = (node**)hDNS;
	for (int i = 0; i < CAPACITY; i++) {
		node* ptr = t[i];
		while (ptr != NULL) {
			node* nextptr = ptr->next;
			free(ptr);
			ptr = nextptr;
		}
	}
	free(t);
}
