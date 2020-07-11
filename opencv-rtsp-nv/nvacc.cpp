#include <opencv2/opencv.hpp>
#include <cstdio>
#include <getopt.h>
#include <sys/time.h>

struct Chrono {
  timeval start;

  Chrono();
  void reset();
  size_t elapsed_ms();
};

Chrono::Chrono() {
  reset();
}

void Chrono::reset() {
  gettimeofday(&start, NULL);
}

size_t Chrono::elapsed_ms() {
  timeval now;
  gettimeofday(&now, NULL);
  return (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000;
}

int main(int argc, char** argv) {
  int opt;
  bool show = false;
  bool hw_acc = false;
  bool write = false;

  while ((opt = getopt(argc, argv, "sHhw")) != -1) {
    switch (opt) {
      case 's':
        show = true;
        break;
      case 'H':
        hw_acc = true;
        break;
      case 'w':
        write = true;
        break;
      case 'h':
      default:
        printf("Usage: %s [-h(elp)] [-s(how)] [-H(W accelerated)] [-w(rite)]\n", *argv);
        return 1;
    }
  }

  std::string gst_r = "rtspsrc location=\"rtsp://***/live/F4859F8E-80D5-4BD4-B5DA-A45E9AA12712\" ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! appsink";
  std::string gst_w = "appsrc ! videoconvert ! video/x-raw,format=I420,width=1280,height=720,framerate=30/1 ! x264enc ! rtph264pay ! udpsink host=127.0.0.1 port=8004";

  if (hw_acc) {
    gst_r = "rtspsrc location=\"rtsp://***/live/F4859F8E-80D5-4BD4-B5DA-A45E9AA12712\" ! rtph264depay ! h264parse ! nvdec ! glcolorconvert ! video/x-raw(memory:GLMemory),format=BGR ! gldownload ! appsink";
    gst_w = "appsrc ! videoconvert ! video/x-raw,format=I420,width=1280,height=720,framerate=30/1 ! nvh264enc ! rtph264pay ! capssetter caps=\"application/x-rtp,profile-level-id=(string)42e01f\" ! udpsink host=127.0.0.1 port=8004";
  }

  cv::VideoCapture cap(gst_r, cv::CAP_GSTREAMER);
  if (!cap.isOpened()){
    printf("Could not open stream\n");
    return 1;
  }

  int fourcc = cv::VideoWriter::fourcc('H', '2', '6', '4');
  cv::Size s(1280, 720);
  cv::VideoWriter writer(gst_w, cv::CAP_GSTREAMER, fourcc, 30, s);

  Chrono chrono;
  int fps = 0;
  double read_ms = 0.;
  double write_ms = 0.;
  double render_ms = 0.;
  while (true) {
    cv::Mat im;
    Chrono read;
    if (!cap.read(im)) {
      printf("Could not read frame\n");
      break;
    }
    read_ms += (double)read.elapsed_ms();
    Chrono render;
    if (show) {
      cv::imshow("Video", im);
      if (cv::waitKey(5) == 27) {
        break;
      }
      render_ms += (double)render.elapsed_ms();
    }
    if (write) {
      Chrono write;
      writer.write(im);
      write_ms += (double)write.elapsed_ms();
    }
    if (chrono.elapsed_ms() >= 1000) {
      printf("-------------------\n");
      printf("fps: %d\n", fps);
      printf("avg read: %.2lfms\n", read_ms/(double)fps);
      if (write) {
        printf("avg write: %.2lfms\n", write_ms/(double)fps);
      }
      if (show) {
        printf("avg render: %.2lfms\n", render_ms/(double)fps);
      }
      fps = 0;
      read_ms = 0.;
      write_ms = 0.;
      render_ms = 0.;
      chrono.reset();
    }
    fps++;
  }
  return 0;
}
