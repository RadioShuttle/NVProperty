
extern int NVPropertyEditor(void);

#ifdef ARDUINO
extern void NVPropertyEditorInit(Stream *serial);
char *ConsoleReadline(char *buf, int buflen, bool echo, int timeout_ms = 0);
#endif
