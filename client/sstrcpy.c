size_t sstrcpy(char *dst, char *src, size_t dest_siz)
{
if (dest_siz < 1)
    return -1;

if (dst == NULL)
    return -1;
if (src == NULL)
    return -1;
memset(dst,0,dest_siz);
strncpy(dst,src,dest_siz -1);
dst[dest_siz -1] = '\0';
return strlen(dst);
}
