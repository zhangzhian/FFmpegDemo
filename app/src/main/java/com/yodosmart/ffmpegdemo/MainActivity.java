package com.yodosmart.ffmpegdemo;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;
import android.text.TextUtils;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;


public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @BindView(R.id.tv_route_1)
    TextView tvRoute1;
    @BindView(R.id.bt_mp4)
    Button btMp4;
    @BindView(R.id.bt1)
    Button bt1;
    @BindView(R.id.tv_route_2)
    TextView tvRoute2;
    @BindView(R.id.bt_avi)
    Button btAvi;
    @BindView(R.id.bt2)
    Button bt2;
    @BindView(R.id.tv_route_3)
    TextView tvRoute3;
    @BindView(R.id.bt_h264)
    Button btH264;
    @BindView(R.id.bt3)
    Button bt3;
    @BindView(R.id.rv)
    RecyclerView rv;
    MyProgressDialog dialog;
    @BindView(R.id.tv_route_4)
    TextView tvRoute4;
    @BindView(R.id.bt_camera)
    Button btCamera;
    @BindView(R.id.bt4)
    Button bt4;
    @BindView(R.id.tv_route_5)
    TextView tvRoute5;
    @BindView(R.id.bt_yuv)
    Button btYuv;
    @BindView(R.id.bt5)
    Button bt5;
    @BindView(R.id.bt6)
    Button bt6;

    private List<String> dataImage = new ArrayList<>();
    private ImageAdapter adapterImage;
    private int picNum;
    private String videoPath;
    Handler handler = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(Message message) {
            if (message.what == 0) {
                dealResult();
            } else if (message.what == 1) {
                Toast.makeText(MainActivity.this, "Error", Toast.LENGTH_SHORT).show();
                dialog.dismiss();
            } else if (message.what == 2) {
                Toast.makeText(MainActivity.this, "成功", Toast.LENGTH_SHORT).show();
                dialog.dismiss();
            }
            return false;
        }
    });

    /**
     * 刷新RV，展示解析的数据
     */
    private void dealResult() {
        for (int i = 0; i < picNum; i++) {
            dataImage.add("/storage/emulated/0/Download/avtest/img/_" + i + ".bmp");
        }
        adapterImage.notifyDataSetChanged();
        dialog.dismiss();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);
        dialog = new MyProgressDialog(this);
        StaggeredGridLayoutManager layoutManager = new StaggeredGridLayoutManager(3, StaggeredGridLayoutManager.VERTICAL);
        rv.setLayoutManager(layoutManager);
        adapterImage = new ImageAdapter(dataImage, this);
        rv.setAdapter(adapterImage);
    }


    @OnClick({R.id.bt_mp4, R.id.bt1, R.id.bt_avi, R.id.bt2, R.id.bt_h264, R.id.bt3, R.id.bt_camera, R.id.bt4, R.id.bt_yuv, R.id.bt5, R.id.bt6})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.bt_mp4:
                openSystemFile(1);
                break;
            case R.id.bt1:
                if (!TextUtils.isEmpty(tvRoute1.getText().toString()))
                    conversion(tvRoute1);
                break;
            case R.id.bt_avi:
                openSystemFile(2);
                break;
            case R.id.bt2:
                if (!TextUtils.isEmpty(tvRoute2.getText().toString()))
                    conversion(tvRoute2);
                break;
            case R.id.bt_h264:
                openSystemFile(3);
                break;
            case R.id.bt3:
                if (!TextUtils.isEmpty(tvRoute3.getText().toString()))
                    conversion(tvRoute3);
                break;
            case R.id.bt_camera:
                camera(4);
                break;
            case R.id.bt4:
                if (!TextUtils.isEmpty(tvRoute4.getText().toString()))
                    conversion(tvRoute4);
                break;
            case R.id.bt_yuv:
                openSystemFile(5);
                break;
            case R.id.bt5:
                if (!TextUtils.isEmpty(tvRoute5.getText().toString()))
                    compress(tvRoute5, 640, 360, 652);
                break;
            case R.id.bt6:
                if (!TextUtils.isEmpty(tvRoute5.getText().toString()))
                    conversionYUV(tvRoute5, 640, 360);
                break;
        }
    }


    /**
     * 调用系统摄像机
     */
    private void camera(int i) {
        videoPath = Environment.getExternalStorageDirectory().getPath() + "/Download/video.mp4";
        Uri fileUri = Uri.fromFile(new File(videoPath));
        Intent intent = new Intent(MediaStore.ACTION_VIDEO_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, fileUri);
        //设置视频录制的最长时间
        intent.putExtra(MediaStore.EXTRA_DURATION_LIMIT, 10);
        //设置视频录制的画质
        intent.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, 1);
        startActivityForResult(intent, i);
    }

    /**
     * 压缩按钮，开始转Mp4
     */
    private void compress(final TextView tvRoute, final int w_jstr, final int h_jstr, final int num_jstr) {
        dialog.showDialog();
        String[] videoInfo = tvRoute.getText().toString().split("/");
        String fileName = videoInfo[videoInfo.length - 1];
        final String[] fileNames = fileName.split("\\.");
        new Thread(new Runnable() {
            public void run() {

                int result = yuvToMp4(tvRoute.getText().toString(), "/storage/emulated/0/Download/avtest/" + fileNames[0] + ".mp4", w_jstr, h_jstr, num_jstr);
                //转码成功
                if (result >= 0) {
                    handler.sendEmptyMessage(2);
                } else {
                    handler.sendEmptyMessage(1);
                }
            }
        }).start();
    }

    /**
     * 确认按钮，开始转码
     */
    private void conversion(final TextView tvRoute) {
        dialog.showDialog();
        String[] videoInfo = tvRoute.getText().toString().split("/");
        String fileName = videoInfo[videoInfo.length - 1];
        String filePath = tvRoute.getText().toString().replace(fileName, "");
        final String[] fileNames = fileName.split("\\.");
        new Thread(new Runnable() {
            public void run() {
                picNum = avToBitmap(tvRoute.getText().toString(), "/storage/emulated/0/Download/avtest/" + fileNames[0] + ".rgb");
                //转码成功
                if (picNum >= 0) {
                    handler.sendEmptyMessage(0);
                } else {
                    handler.sendEmptyMessage(1);
                }
            }
        }).start();
    }

    /**
     * 把YUV格式的视频解析成BMP
     */
    private void conversionYUV(final TextView tvRoute, final int w_jstr, final int h_jstr) {
        dialog.showDialog();
        String[] videoInfo = tvRoute.getText().toString().split("/");
        String fileName = videoInfo[videoInfo.length - 1];
        final String[] fileNames = fileName.split("\\.");
        new Thread(new Runnable() {
            public void run() {

                picNum = yuvToBitmap(tvRoute.getText().toString(), "/storage/emulated/0/Download/avtest/" + fileNames[0] + ".rgb", w_jstr, h_jstr);
                //成功
                if (picNum >= 0) {
                    handler.sendEmptyMessage(0);
                } else {
                    handler.sendEmptyMessage(1);
                }
            }
        }).start();
    }


    /**
     * 调用系统文件管理器
     *
     * @param type
     */
    public void openSystemFile(int type) {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        //系统调用Action属性
        intent.setType("*/*");
        //设置文件类型
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        // 添加Category属性
        try {
            startActivityForResult(intent, type);
        } catch (Exception e) {
            Toast.makeText(this, "没有正确打开文件管理器", Toast.LENGTH_SHORT).show();
        }
    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == Activity.RESULT_OK) {//是否选择，没选择就不会继续
            if (requestCode == 4) {
                tvRoute4.setText(videoPath);
            } else {
                Uri uri = data.getData();//得到uri，后面就是将uri转化成file的过程。
                String[] proj = {MediaStore.Images.Media.DATA};
                Cursor actualimagecursor = managedQuery(uri, proj, null, null, null);
                int actual_image_column_index = actualimagecursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
                actualimagecursor.moveToFirst();
                String img_path = actualimagecursor.getString(actual_image_column_index);
                if (img_path.endsWith(".avi") || img_path.endsWith(".mp4") || img_path.endsWith(".h264") || img_path.endsWith(".yuv")) {
                    if (requestCode == 1) {
                        tvRoute1.setText(img_path);
                    } else if (requestCode == 2) {
                        tvRoute2.setText(img_path);
                    } else if (requestCode == 3) {
                        tvRoute3.setText(img_path);
                    } else if (requestCode == 5) {
                        tvRoute5.setText(img_path);
                    }
                } else {
                    Toast.makeText(this, "请选择正确的文件", Toast.LENGTH_SHORT).show();
                }
            }
        }
    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();


    public native String urlprotocolinfo();

    public native String avformatinfo();

    public native String avcodecinfo();

    public native String avfilterinfo();

    /**
     * avi/mp4/h264转换成Bitmap
     *
     * @param input_jstr  输入的avi/mp4/h264路径
     * @param output_jstr 输出的rgb路径
     * @return 大于等于0解析正确，返回的是解析生成bmp的数量
     * 小于0解析出现问题
     */
    public native int avToBitmap(String input_jstr, String output_jstr);

    public native int RGBToBitmap();

    /**
     * yuv转换成mp4
     *
     * @param input_jstr  输入的路径
     * @param output_jstr 输出路径
     * @param w_jstr      宽
     * @param h_jstr      高
     * @param num_jstr    帧数量
     * @return
     */
    public native int yuvToMp4(String input_jstr, String output_jstr, int w_jstr, int h_jstr, int num_jstr);

    /**
     * yuv转换成bmp
     * @param input_jstr 输入的路径
     * @param output_jstr 输出路径
     * @param w_jstr 宽
     * @param h_jstr 高
     * @return
     */
    public native int yuvToBitmap(String input_jstr, String output_jstr, int w_jstr, int h_jstr);

}
