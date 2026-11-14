/* Firmware simulé pour mqtt_selftest.sh : ht_* appelés par md_inject.c. Journal sur la sortie standard. */
#include <stdio.h>
#include <unistd.h>
int ht_montion_detect_set_active_cb(void (*cb)(long long)) { (void)cb; return 0; }
int ht_montion_detect_open(void) { return 0; }
int ht_montion_detect_set_sensitivity(int l) { (void)l; return 0; }
int ht_montion_detect_set_human_filter(int e) { (void)e; return 0; }
int ht_night_mode_set_irled_mode(int m) { printf("FAKE irled %d\n", m); fflush(stdout); return 0; }
int system(const char *c) { printf("FAKE system %s\n", c); fflush(stdout); return 0; }
int main(void) { for (;;) pause(); }
