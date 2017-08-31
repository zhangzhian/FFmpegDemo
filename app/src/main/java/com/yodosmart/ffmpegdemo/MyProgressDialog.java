package com.yodosmart.ffmpegdemo;

import android.app.Dialog;
import android.content.Context;
import android.view.Window;

import com.wang.avi.AVLoadingIndicatorView;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MyProgressDialog extends Dialog {

    @BindView(R.id.avi)
    AVLoadingIndicatorView avi;
    private Context context;


    public MyProgressDialog(Context context) {
        super(context, R.style.ActionDialogStyle);
        this.context = context;
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.dialog_progress);
        ButterKnife.bind(this);
        initView();
    }

    private void initView() {
        avi.show();
//         or avi.smoothToShow();
    }

    public void showDialog() {
        setCanceledOnTouchOutside(false);// 设置点击屏幕Dialog不消失
        show();
    }

}
