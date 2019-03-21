#include "Mapping.h"

#include <fstream>
#include <iostream>

#ifdef USE_DETAIL
#include <zzip/zzip.h>
#include "src/frame.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#endif

void Mapping::load_samples(const char *name, Filer& filer) {
  if (name == NULL) { return; }
#ifdef USE_DETAIL
  have_sum = true;

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  makesweet::Frame frame;


  ZZIP_DIR *zip = (ZZIP_DIR *)filer.getZip();
  if (!zip) {
    fprintf(stderr, "no zip file\n");
    exit(1);
  }
  ZZIP_FILE *fp = NULL;
  fp = zzip_file_open(zip, name, ZZIP_CASELESS);
  if (!fp) {
    fprintf(stderr,"Cannot open %s in zip\n", name);
    exit(1);
  }
  char buf[10000];
  std::string data;
  while (true) {
    zzip_ssize_t len = zzip_file_read(fp, buf, 10000);
    if (!len) {
      break;
    }
    data += std::string(buf, len);
  }
  zzip_file_close(fp);

  // printf("Loading samples %s starting\n", name);
  google::protobuf::io::ArrayInputStream input_stream(data.data(), (int)data.size());
  google::protobuf::io::CodedInputStream coded_stream(&input_stream);
  coded_stream.SetTotalBytesLimit(2147483647, -1);
  // if (!frame.ParseFromString(data)) {
  if (!frame.ParseFromCodedStream(&coded_stream)) {
    fprintf(stderr, "cannot load samples %s\n", name);
    exit(1);
  }
  // std::cout << frame.DebugString() << std::endl;
  float min_fx = frame.min_fx();
  float max_fx = frame.max_fx();
  float min_fy = frame.min_fy();
  float max_fy = frame.max_fy();
  float fscale = frame.fscale();
  // printf("Loading samples %s summarizing\n", name);
  for (auto &summary: frame.summaries()) {
    int x = summary.x();
    int y = summary.y();
    Summary& lsummary = sum.summaries[sum.offset(x, y)];
    /*
    printf("samples %d\n", (int)summary.samples().size());
    if (summary.samples().size() == 80) {
      for (auto &sample: summary.samples()) {
        printf("%.6f %.6f %.2f   ", sample.fx(), sample.fy(), sample.factor());
      }
      printf("\n");
    }
    */
    for (auto &sample: summary.samples()) {
      float fx = ((sample.fx() / fscale) * (max_fx - min_fx) + min_fx);
      float fy = ((sample.fy() / fscale) * (max_fy - min_fy) + min_fy);
      float factor = (float)sample.factor();
      // printf(" %g %g %g\n", (double) fx, (double) fy, (double) factor);
      lsummary.samples.push_back(Sample({fx, fy, factor}));
    }
  }
  // printf("Loading samples %s done\n", name);
#endif
}
