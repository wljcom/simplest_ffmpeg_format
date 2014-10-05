/**
 * ��򵥵Ļ���FFmpeg�ķ�װ��ʽת����
 * Simplest FFmpeg Remuxer
 *
 * ������ Lei Xiaohua
 * leixiaohua1020@126.com
 * �й���ý��ѧ/���ֵ��Ӽ���
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * ������ʵ������Ƶ��װ��ʽ֮���ת����
 * ��Ҫע����Ǳ����򲢲��ı�����Ƶ�ı����ʽ��
 * �������޸���FFmpeg��tutorial��
 *
 * This software converts a media file from one container format
 * to another container format without encoding/decoding video files.
 * This software is modified from FFmpeg's tutorial.
 */

#include <stdio.h>

extern "C"
{
#include "libavformat/avformat.h"
};


int main(int argc, char* argv[])
{
	AVOutputFormat *ofmt = NULL;
	//�����Ӧһ��AVFormatContext�������Ӧһ��AVFormatContext
	//��Input AVFormatContext and Output AVFormatContext��
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;
	const char *in_filename, *out_filename;
	int ret, i;

	in_filename  = "cuc_ieschool1.flv";//�����ļ�����Input file URL��
	out_filename = "cuc_ieschool1.mp4";//����ļ�����Output file URL��
	av_register_all();
	//���루Input��
	if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		printf( "Could not open input file.");
		goto end;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		printf( "Failed to retrieve input stream information");
		goto end;
	}
	av_dump_format(ifmt_ctx, 0, in_filename, 0);
	//�����Output��
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
	if (!ofmt_ctx) {
		printf( "Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		//���������������������Create output AVStream according to input AVStream��
		AVStream *in_stream = ifmt_ctx->streams[i];
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		if (!out_stream) {
			printf( "Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		//����AVCodecContext�����ã�Copy the settings of AVCodecContext��
		if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
			printf( "Failed to copy context from input to output stream codec context\n");
			goto end;
		}
		out_stream->codec->codec_tag = 0;
		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	//���һ�¸�ʽ------------------
	av_dump_format(ofmt_ctx, 0, out_filename, 1);
	//������ļ���Open output file��
	if (!(ofmt->flags & AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
		if (ret < 0) {
			printf( "Could not open output file '%s'", out_filename);
			goto end;
		}
	}
	//д�ļ�ͷ��Write file header��
	if (avformat_write_header(ofmt_ctx, NULL) < 0) {
		printf( "Error occurred when opening output file\n");
		goto end;
	}
	int frame_index=0;
	while (1) {
		AVStream *in_stream, *out_stream;
		//��ȡһ��AVPacket��Get an AVPacket��
		ret = av_read_frame(ifmt_ctx, &pkt);
		if (ret < 0)
			break;
		in_stream  = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];
		/* copy packet */
		//ת��PTS/DTS��Convert PTS/DTS��
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		//д�루Write��
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			printf( "Error muxing packet\n");
			break;
		}
		printf("Write %8d frames to output file\n",frame_index);
		av_free_packet(&pkt);
		frame_index++;
	}
	//д�ļ�β��Write file trailer��
	av_write_trailer(ofmt_ctx);
end:
	avformat_close_input(&ifmt_ctx);
	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	if (ret < 0 && ret != AVERROR_EOF) {
		printf( "Error occurred.\n");
		return -1;
	}
	return 0;
}
