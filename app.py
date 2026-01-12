import os
import urllib
import datetime
import subprocess
import numpy as np
import pandas as pd
import streamlit as st


def run_calc(v_path, h_path, visitor_name, home_name, i, t, w, s, r1, r2, r3, b1, b2):
    try:
        # C++側に渡す数値文字列（スペース区切り）
        # ※r1, r2, r3, b1, b2 の順番はC++の cin と完全に一致させてください
        input_str = f"{i} {t} {w} {s} {r1} {r2} {r3} {b1} {b2}\n"

        # ファイルの準備
        with open(v_path, "r") as f:
            v_data = f.read()
        with open(h_path, "r") as f:
            h_data = f.read()

        os.makedirs("data", exist_ok=True)
        with open("data/top.txt", "w") as f:
            f.write(v_data)
        with open("data/bottom.txt", "w") as f:
            f.write(h_data)

        # --- 計算実行の修正箇所 ---
        # shell=True を使うか、Popenで標準入力を渡します
        result = subprocess.run(
            ["./calc"],                # 実行ファイル名
            input=input_str,           # 標準入力として渡す文字列
            capture_output=True,       # 出力を受け取る
            text=True,                 # 文字列として扱う
            check=True                 # エラー時に例外を出す
        )
        
        output_raw = result.stdout
        lines = output_raw.strip().split("\n")
        
        # C++側の出力形式に合わせてパース
        # 例: "Visitor:0.55" のような形式を想定
        res_v = lines[0].split(":")[1] if ":" in lines[0] else lines[0]
        res_h = lines[1].split(":")[1] if len(lines) > 1 and ":" in lines[1] else "---"
        
        return res_v, res_h

    except subprocess.CalledProcessError as e:
        return f"C++ Error", f"{e.stderr}"
    except Exception as e:
        return f"Error", f"{str(e)}"


if __name__ == "__main__":

    # --- サイドバーに試合状況の設定スペースを作成 ---
    with st.sidebar:
        st.header("試合状況の設定")

        # イニングと表裏
        inning = st.selectbox("イニング", range(1, 13), index=0) - 1  # 1回〜12回
        top_bottom = st.radio("表 / 裏", ["表 (先攻攻撃)", "裏 (後攻攻撃)"])
        t_val = 0 if top_bottom == "表 (先攻攻撃)" else 1

        # アウトカウント
        outs = st.selectbox("アウトカウント", [0, 1, 2], index=0)

        # ランナー状況
        st.write("**ランナー**")
        r1 = st.checkbox("一塁ランナーあり", value=False)
        r2 = st.checkbox("二塁ランナーあり", value=False)
        r3 = st.checkbox("三塁ランナーあり", value=False)
        r1 = 1 if r1 else 0
        r2 = 1 if r2 else 0
        r3 = 1 if r3 else 0

        # スコア差 (s - INITIAL_SCORE に相当する値)
        # 0なら同点、プラスなら先攻リード、マイナスなら後攻リード
        score_diff = st.number_input(
            "スコア差 (先攻基準)",
            value=0,
            help="例: 1 = 先攻1点リード, -2 = 後攻2点リード",
        )

        # C++側に渡すための s の計算
        # initial_score = 20
        selected_score = 20 + score_diff

        # 打順 (r1:先攻打順, r2:後攻打順) ※必要に応じて
        b1 = st.number_input("先攻の現在の打順 (1-9)", 1, 9, 1) - 1
        b2 = st.number_input("後攻の現在の打順 (1-9)", 1, 9, 1) - 1

    game_date = datetime.date(2025, 4, 3)
    st.title(f"{str(game_date)}のプロ野球勝率予測")
    date_str = game_date.strftime("%Y-%m-%d")
    base_path = f"data/{date_str}"

    if not os.path.exists(base_path):
        st.error(f"ディレクトリが見つかりません: {base_path}")
    else:
        game_dirs = sorted(
            [
                d
                for d in os.listdir(base_path)
                if os.path.isdir(os.path.join(base_path, d))
            ]
        )

        if not game_dirs:
            st.warning("試合データが存在しません。")
        else:
            game_labels = []
            game_info = []

            for g_id in game_dirs:
                g_path = os.path.join(base_path, g_id)
                teams = [f for f in os.listdir(g_path) if f.endswith(".txt")]
                if len(teams) >= 2:
                    t1 = teams[0].replace(".txt", "")
                    t2 = teams[1].replace(".txt", "")
                    game_labels.append(f"{t1} vs {t2}")
                    game_info.append(
                        {
                            "teams": [t1, t2],
                            "paths": [
                                os.path.join(g_path, teams[0]),
                                os.path.join(g_path, teams[1]),
                            ],
                        }
                    )

            if game_labels:
                # 3. 試合の選択
                selected_game_label = st.selectbox(
                    "試合を選択してください", game_labels
                )
                i = game_labels.index(selected_game_label)
                current_game = game_info[i]

                # --- 【追加】先攻・後攻の選択ロジック ---
                st.write("---")
                col_sel1, col_sel2 = st.columns(2)

                # 選択肢としてチーム名のリストを作成
                team_options = current_game["teams"]

                with col_sel1:
                    visitor_name = st.selectbox(
                        "先攻チームを選択", team_options, index=0
                    )
                with col_sel2:
                    # 先攻に選ばれなかった方をデフォルトにする
                    default_home_idx = 1 if team_options.index(visitor_name) == 0 else 0
                    home_name = st.selectbox(
                        "後攻チームを選択", team_options, index=default_home_idx
                    )

                if visitor_name == home_name:
                    st.error("先攻と後攻には別のチームを選択してください。")
                else:
                    # パスを確定させる
                    v_path = current_game["paths"][team_options.index(visitor_name)]
                    h_path = current_game["paths"][team_options.index(home_name)]

                    # スタメンデータのプレビュー
                    try:
                        v_df = pd.read_csv(v_path, sep=" ")
                        h_df = pd.read_csv(h_path, sep=" ")

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
                            res_v, res_h = run_calc(
                                v_path,
                                h_path,
                                visitor_name,
                                home_name,
                                inning,
                                t_val,
                                outs,
                                selected_score,
                                r1,
                                r2,
                                r3,
                                b1,
                                b2,
                            )

                            st.divider()
                            st.subheader("予測結果")
                            m1, m2 = st.columns(2)
                            m1.metric(f"{visitor_name} (先攻) 期待勝率", res_v)
                            m2.metric(f"{home_name} (後攻) 期待勝率", res_h)
            else:
                st.info("対戦データが不足しています。")
