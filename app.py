import os
import urllib
import datetime
import subprocess
import numpy as np
import pandas as pd
import streamlit as st


def run_calc(v_path, h_path, visitor_name, home_name):
    """
    指定されたパスのファイルを読み込み、calcに渡して勝率を計算する
    """
    # 1. C++が読み込める場所にファイルをコピー（または中身を読み替えて出力）
    # ここでは単純にファイルを読み込んで ./data/top.txt, ./data/bottom.txt に保存し直す例
    try:
        with open(v_path, 'r') as f:
            v_data = f.read()
        with open(h_path, 'r') as f:
            h_data = f.read()
            
        os.makedirs("data", exist_ok=True)
        with open("data/top.txt", "w") as f:
            f.write(v_data)
        with open("data/bottom.txt", "w") as f:
            f.write(h_data)

        # 2. 計算実行
        cmd = "./calc"
        output_raw = subprocess.check_output(cmd).decode()
        lines = output_raw.split("\n")[:2]
        
        res_v = lines[0].split(":")[1]
        res_h = lines[1].split(":")[1]
        return res_v, res_h
    except Exception as e:
        return f"Error: {e}", f"Error: {e}"


if __name__ == "__main__":
    game_date = datetime.date(2025, 4, 3)
    st.title(f"{str(game_date)}のプロ野球勝率予測")
    date_str = game_date.strftime("%Y-%m-%d")
    base_path = f"data/{date_str}"

    if not os.path.exists(base_path):
        st.error(f"ディレクトリが見つかりません: {base_path}")
    else:
        game_dirs = sorted([d for d in os.listdir(base_path) if os.path.isdir(os.path.join(base_path, d))])
        
        if not game_dirs:
            st.warning("試合データが存在しません。")
        else:
            game_labels = []
            game_info = []

            for g_id in game_dirs:
                g_path = os.path.join(base_path, g_id)
                teams = [f for f in os.listdir(g_path) if f.endswith('.txt')]
                if len(teams) >= 2:
                    t1 = teams[0].replace('.txt', '')
                    t2 = teams[1].replace('.txt', '')
                    game_labels.append(f"{t1} vs {t2}")
                    game_info.append({
                        "teams": [t1, t2],
                        "paths": [os.path.join(g_path, teams[0]), os.path.join(g_path, teams[1])]
                    })

            if game_labels:
                # 3. 試合の選択
                selected_game_label = st.selectbox("試合を選択してください", game_labels)
                i = game_labels.index(selected_game_label)
                current_game = game_info[i]

                # --- 【追加】先攻・後攻の選択ロジック ---
                st.write("---")
                col_sel1, col_sel2 = st.columns(2)
                
                # 選択肢としてチーム名のリストを作成
                team_options = current_game["teams"]
                
                with col_sel1:
                    visitor_name = st.selectbox("先攻チームを選択", team_options, index=0)
                with col_sel2:
                    # 先攻に選ばれなかった方をデフォルトにする
                    default_home_idx = 1 if team_options.index(visitor_name) == 0 else 0
                    home_name = st.selectbox("後攻チームを選択", team_options, index=default_home_idx)

                if visitor_name == home_name:
                    st.error("先攻と後攻には別のチームを選択してください。")
                else:
                    # パスを確定させる
                    v_path = current_game["paths"][team_options.index(visitor_name)]
                    h_path = current_game["paths"][team_options.index(home_name)]

                    # スタメンデータのプレビュー
                    try:
                        v_df = pd.read_csv(v_path, sep=' ')
                        h_df = pd.read_csv(h_path, sep=' ')
                        
                        # 先攻チームを表示
                        st.subheader(f"先攻: {visitor_name}")
                        st.dataframe(v_df, width="stretch", hide_index=True)
                        
                        # 後攻チームを表示（縦に並ぶ）
                        st.subheader(f"後攻: {home_name}")
                        st.dataframe(h_df, width="stretch", hide_index=True)
                    except:
                        st.caption("データプレビューを表示できません。")

                    # 4. 勝率計算ボタン
                    if st.button("勝率を計算", key=f"calc_{i}"):
                        with st.spinner("計算エンジン実行中..."):
                            # 選択された通りの順番でrun_calcに渡す
                            res_v, res_h = run_calc(v_path, h_path, visitor_name, home_name)
                            
                            st.divider()
                            st.subheader("予測結果")
                            m1, m2 = st.columns(2)
                            m1.metric(f"{visitor_name} (先攻) 期待勝率", res_v)
                            m2.metric(f"{home_name} (後攻) 期待勝率", res_h)
            else:
                st.info("対戦データが不足しています。")