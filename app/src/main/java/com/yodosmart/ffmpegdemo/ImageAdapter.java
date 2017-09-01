package com.yodosmart.ffmpegdemo;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.bumptech.glide.Glide;
import com.bumptech.glide.load.engine.DiskCacheStrategy;

import java.io.File;
import java.util.List;

public class ImageAdapter extends RecyclerView.Adapter<ImageAdapter.ViewHolder> {

    private List<String> data;
    private Context context;
    private View view;

    static class ViewHolder extends RecyclerView.ViewHolder {
        ImageView iv_image;

        public ViewHolder(View view) {
            super(view);
            iv_image = (ImageView) view.findViewById(R.id.iv_image);
        }
    }

    public ImageAdapter(List<String> data, Context context) {
        this.data = data;
        this.context = context;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        view = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_image, parent, false);
        ViewHolder holder = new ViewHolder(view);
        return holder;
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        position = holder.getAdapterPosition();
        if (data.size() > 0) {
            //本地文件
            File file = new File(data.get(position));
            //加载图片
            Glide.with(context).load(file).skipMemoryCache(true).diskCacheStrategy( DiskCacheStrategy.NONE ).into(holder.iv_image);
            holder.iv_image.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public int getItemCount() {
        return data.size();
    }

}
