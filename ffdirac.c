#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

static int process_frames(AVFormatContext *formatCtx, AVCodecContext *codecCtx,
		int sn, AVFrame *frame)
{
	int ret = 0;

	// Create "packet" structure for compressed data.  NULL buffer will be
	// automatically managed by libavcodec.
	AVPacket pkt;
	pkt.buf = NULL;

	// Process each packet in turn
	while ((ret = av_read_frame(formatCtx, &pkt)) >= 0) {
		// Only care about one stream
		if (pkt.stream_index != sn)
			continue;

		// Packet may require multiple calls to decode (e.g. compressed
		// packet containing multiple uncompressed frames)
		while (pkt.size > 0) {
			// Decode packet to frame
			int n, got_frame;
			n = avcodec_decode_audio4(codecCtx, frame, &got_frame, &pkt);
			if (n < 0) {
				fprintf(stderr, "Error in decoding\n");
				return 1;
			}

			// A packet may contain only part of a frame, so we may
			// not get a frame with every call to decode
			if (got_frame) {
				// XXX handle audio data here
				fprintf(stderr,"frame\n");
			}

			pkt.size -= n;
			pkt.data += n;
		}

		if (pkt.size != 0) {
			fprintf(stderr, "Read more than size of packet?!\n");
			return 1;
		}
	}

	// EOF is the normal way to terminate this loop
	return (ret != AVERROR_EOF);
}

static int process_stream(AVFormatContext *formatCtx, AVCodecContext *codecCtx,
		int sn)
{
	int ret = 0;

	// Allocate "frame" structure for uncompressed data
	AVFrame *frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Couldn't allocate frame buffer\n");
		return 1;
	}

	ret = process_frames(formatCtx, codecCtx, sn, frame);

	av_frame_free(&frame);

	return ret;
}

static int process_file(AVFormatContext *formatCtx)
{
	int ret = 0;

	// Load information about streams
	if (avformat_find_stream_info(formatCtx, NULL) < 0) {
		fprintf(stderr, "Couldn't get stream info\n");
		return 1;
	}

	// Debug: print file info
	av_dump_format(formatCtx, 0, "file", 0);

	// Open appropriate codec for default audio stream
	AVCodec *codec;
	int sn = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1,
				&codec, 0);
	if (sn < 0) {
		fprintf(stderr, "Couldn't find audio stream\n");
		return 1;
	}

	// Initialize codec context
	AVCodecContext *codecCtx = formatCtx->streams[sn]->codec;
	if (avcodec_open2(codecCtx, codec, NULL) < 0) {
		fprintf(stderr, "Couldn't open codec context\n");
		return 1;
	}

	ret = process_stream(formatCtx, codecCtx, sn);

	avcodec_close(codecCtx);

	return ret;
}

int main(int argc, char **argv)
{
	int ret = 0;

	if (argc < 2) {
		fprintf(stderr, "usage: %s file\n", argv[0]);
		return 1;
	}

	// Set up avcodec
	av_register_all();

	// Open file
	AVFormatContext *formatCtx = NULL;
	if (avformat_open_input(&formatCtx, argv[1], NULL, NULL) < 0) {
		fprintf(stderr, "Couldn't open input file %s\n", argv[1]);
		return 1;
	}

	ret = process_file(formatCtx);

	// Close file
	avformat_close_input(&formatCtx);

	return ret;
}
