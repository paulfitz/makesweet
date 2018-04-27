#ifdef __cplusplus
extern "C" {
#endif

  extern void *pf_gdQuantizeInit (gdImagePtr oim, gdImagePtr nim);
  extern void pf_gdQuantizeApply (void *v, gdImagePtr oim, int dither, gdImagePtr nim);
  extern void pf_gdQuantizeFini (void *v);

  //#define pf_gdQuantizeInit gdQuantizeInit
  //#define pf_gdQuantizeApply gdQuantizeApply
  //#define pf_gdQuantizeFini gdQuantizeFini

#ifdef __cplusplus
}
#endif
