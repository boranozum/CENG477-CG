all:
	g++ -o hw3 main.cpp utils.cpp find.cpp read.cpp create.cpp move.cpp

mount:
	fusefat -o rw+ -o umask=770 example.img fs-root

unmount:
	fusermount -u fs-root

info:
	fsck.vfat -vn example.img

image:
	mkdir fs-root
	dd if=/dev/zero of=example.img bs=1024 count=102400
	mkfs.vfat -F 32 -S 512 -s 2 example.img

clean:
	rm -rf fs-root
	rm example.img

ci: clean image

test:
	mkdir fs-root/b1
	mkdir fs-root/b2
	mkdir fs-root/b3
	mkdir fs-root/b4
	mkdir fs-root/b5
	mkdir fs-root/b6
	mkdir fs-root/b7
	mkdir fs-root/b8
	mkdir fs-root/b9
	mkdir fs-root/b10
	mkdir fs-root/b11
	mkdir fs-root/b12
	mkdir fs-root/b13
	mkdir fs-root/b14
	mkdir fs-root/b15
