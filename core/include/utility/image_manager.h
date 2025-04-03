#ifndef IMAGE_MANAGER_
#define IMAGE_MANAGER_

typedef struct {
	unsigned char bits_per_channel, channel;
	unsigned short width, height;
	unsigned char *bitmap;
} im_image;
typedef enum {
	NONE, // NULL purpose
	PNG
} im_image_format;

extern im_image im_read_image_from_mem(const unsigned char *, unsigned int);
extern im_image im_read_image_from_file(const char *);

extern void *im_write_image_to_mem(im_image, im_image_format, unsigned int *);
extern int im_write_image_to_file(const char *,im_image, im_image_format);



extern void im_free_image(im_image);
#endif // IMAGE_MANAGER_