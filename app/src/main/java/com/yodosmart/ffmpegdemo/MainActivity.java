package com.yodosmart.ffmpegdemo;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

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

    private List<String> dataImage = new ArrayList<>();
    private ImageAdapter adapterImage;
    private int picNum;
    Handler handler = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(Message message) {
            if (message.what == 0) {
                dealResult();
            } else if (message.what == 1) {
                Toast.makeText(MainActivity.this, "Error", Toast.LENGTH_SHORT).show();
                dialog.dismiss();
            }
            return false;
        }
    });

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


    @OnClick({R.id.bt_mp4, R.id.bt1, R.id.bt_avi, R.id.bt2, R.id.bt_h264, R.id.bt3})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.bt_mp4:
                openSystemFile(1);
                break;
            case R.id.bt1:
                ensure(tvRoute1);
                break;
            case R.id.bt_avi:
                openSystemFile(2);
                break;
            case R.id.bt2:
                ensure(tvRoute2);
                break;
            case R.id.bt_h264:
                openSystemFile(3);
                break;
            case R.id.bt3:
                ensure(tvRoute3);
                break;
        }
    }

    /**
     * 确认按钮，开始转码
     */
    private void ensure(final TextView tvRoute) {
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
            Uri uri = data.getData();//得到uri，后面就是将uri转化成file的过程。
            String[] proj = {MediaStore.Images.Media.DATA};
            Cursor actualimagecursor = managedQuery(uri, proj, null, null, null);
            int actual_image_column_index = actualimagecursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
            actualimagecursor.moveToFirst();
            String img_path = actualimagecursor.getString(actual_image_column_index);
            if (img_path.endsWith(".avi") || img_path.endsWith(".mp4") || img_path.endsWith(".h264")) {
                if (requestCode == 1) {
                    tvRoute1.setText(img_path);
                } else if (requestCode == 2) {
                    tvRoute2.setText(img_path);
                } else if (requestCode == 3) {
                    tvRoute3.setText(img_path);
                }
            } else {
                Toast.makeText(this, "请选择正确的文件", Toast.LENGTH_SHORT).show();
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

    public native int avToBitmap(String input_jstr, String output_jstr);

    public native int RGBToBitmap();

}
