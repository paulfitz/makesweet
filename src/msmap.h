#ifndef HAVE_MSMAP_H
#define HAVE_MSMAP_H

#ifdef __cplusplus
extern "C" {
#endif

  void msmap_enable();
  void msmap_set_frame(int frame);
  void msmap_set_aux(int is_aux);
  void msmap_set_output(int rx, int ry);
  void msmap_unset_output();
  void msmap_set_input(int index, float fx, float fy);
  void msmap_set_bounce(int bounce);
  int msmap_parse_name(char *name);
  int msmap_set_index(int external_index, int internal_index);
  void msmap_render(int w, int h);
  void msmap_render_scaled(int w, int h, int factor);

#ifdef __cplusplus
}
#endif


#endif
