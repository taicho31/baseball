import os 
import urllib
import requests
import datetime
import subprocess
import numpy as np
import pandas as pd
from bs4 import BeautifulSoup

giants_url = "https://baseballdata.jp/1/ctop.html"
swallows_url = "https://baseballdata.jp/2/ctop.html"
dena_url = "https://baseballdata.jp/3/ctop.html"
dragons_url = "https://baseballdata.jp/4/ctop.html"
tigers_url = "https://baseballdata.jp/5/ctop.html"
carp_url = "https://baseballdata.jp/6/ctop.html"

lions_url = "https://baseballdata.jp/7/ctop.html"
fighters_url = "https://baseballdata.jp/8/ctop.html"
lotte_url = "https://baseballdata.jp/9/ctop.html"
orix_url = "https://baseballdata.jp/11/ctop.html"
hawks_url = "https://baseballdata.jp/12/ctop.html"
eagles_url = "https://baseballdata.jp/376/ctop.html"

data_path = "./data/"

team_dic = {
        '読売ジャイアンツ':  giants_url ,  
        '東京ヤクルトスワローズ': swallows_url, 
        '横浜DeNAベイスターズ': dena_url, 
        '中日ドラゴンズ': dragons_url, 
        '阪神タイガース': tigers_url, 
        '広島東洋カープ': carp_url,
                           
        '埼玉西武ライオンズ': lions_url, 
        '北海道日本ハムファイターズ': fighters_url, 
        '千葉ロッテマリーンズ': lotte_url, 
        'オリックス・バファローズ': orix_url, 
        '福岡ソフトバンクホークス': hawks_url, 
        '東北楽天ゴールデンイーグルス': eagles_url
    }


# dateで受け取った日に開催された各試合の出場成績のリンクをリスト形式で返す
def get_top_links(date):
    params = { 'date': date }
    schedule_page = requests.get('https://baseball.yahoo.co.jp/npb/schedule', params=params)
    soup_schedule = BeautifulSoup(schedule_page.text, 'html.parser')
    game_link_elms = soup_schedule.find_all('a', class_='bb-score__content')
    game_links = list(map(lambda x: x['href'].replace('index', 'top'), game_link_elms))
    return game_links


def data_generate(URL, debug=False):
    html = urllib.request.urlopen(URL)

    # htmlをBeautifulSoupで扱う
    soup = BeautifulSoup(html, "html.parser")
    
    tmp_data= []
    web_data = soup.tbody.find_all("tr")
    for i in range(len(web_data)):
        row_tmp = web_data[i].getText().replace("\r", "").replace(" ", "").split("\n")
        row = [a for a in row_tmp if a != '']
        if row not in tmp_data:
            if "○" in row: #一軍にいる選手
                row.pop(2) #一軍のoマークを削除
                tmp_data.append(row) 
            elif "選手名" in row:
                title = row
            else:
                tmp_data.append(row) #一軍にいないとされている選手
    
    title.remove("調子")
    title.remove("一軍")

    df = pd.DataFrame(tmp_data)    
    df.columns = title
    team = df.iloc[0]["球団"]
        
    # データの型をobject型からintまたはfloat型に変換する
    for i in ["打点", "本塁打", "安打数", "単打", "2塁打", "3塁打", "併殺", "四球", "打席数", "打数",
              "死球", "企盗塁", "盗塁", "企犠打","犠打","犠飛",]:
        df[i] = df[i].astype("int")

    for i in ["打率"]:
        df[i] = df[i].astype("float")

    df["盗塁成功率"] = df["盗塁"] / df["企盗塁"]
    df["犠打成功率"] = df["犠打"] / df["企犠打"]
    
    # 欠損値は0で補完
    df["盗塁成功率"] = df["盗塁成功率"].fillna(0)
    df["犠打成功率"] = df["犠打成功率"].fillna(0)

    #　選手名を修正
    df["選手名"] = df["選手名"].apply(lambda x: x.split(":")[1].split(".")[0].split(team)[0])
    
    # 各選手について 凡退,単打率 二塁打率 三塁打率 本塁打率 四死球率 盗塁成功率 犠打成功率　併殺打率を計算する
    denominator = (df["打席数"] - df["犠打"] - df["犠飛"])
    df["単打率"] = df["単打"] / denominator
    df["二塁打率"] = df["2塁打"] / denominator
    df["三塁打率"] = df["3塁打"] / denominator
    df["本塁打率"] = df["本塁打"] / denominator
    df["四死球率"] = (df["四球"] + df["死球"]) / denominator
    df["併殺打率"] = df["併殺"] / denominator
    df["凡退率"] =  (denominator - df["単打"] - df["2塁打"] - df["3塁打"] - df["本塁打"] - df["四球"] - df["死球"]) / denominator
    df["併殺打率"] = df["併殺"] / denominator

    calc_df = df[["選手名", "凡退率","単打率","二塁打率","三塁打率","本塁打率","四死球率","盗塁成功率","犠打成功率", "併殺打率"]]
    return calc_df, team


# https://stackoverflow.com/questions/31247198/python-pandas-write-content-of-dataframe-into-text-file
def make_df(url, players, opt):
    df, team = data_generate(url)
    players = {i:ind for ind, i in enumerate(players)}
    inv_players = {ind:i for ind, i in enumerate(players)}
    
    df['order'] = df['選手名'].map(players)
    
    #今季一軍初出場選手の調整
    missing_no = list(set(range(9)) - set(df.order.unique()))
    missing_players = [inv_players[i] for i in missing_no] 
    
    for i in range(len(missing_players)):
        tmp = [1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.0, 0.000000, 0.000000]
        tmp = pd.DataFrame([missing_players[i]] + tmp + [missing_no[i]]).T
        tmp.columns = df.columns
        df = pd.concat([df, tmp], axis=0)
    
    df = df[~df.order.isnull()].sort_values("order", ascending=True).drop('order', axis=1).reset_index(drop=True)
    
    # 値が欠損の選手の調整
    df["凡退率"] = df["凡退率"].fillna(1.0)
    df = df.fillna(0)
        
    assert df.shape[0] == 9
    assert np.allclose(df.iloc[:,1:7].sum(axis=1), 1)

    print(df)
    
    return df


if __name__ == "__main__":

    if not os.path.isdir(data_path):
        os.mkdir(data_path)
    
    game_date = datetime.date(2025,4,3)
    game_links = get_top_links(game_date)

    if not os.path.isdir(data_path+str(game_date)):
        os.mkdir(data_path+str(game_date))
    
    for game_id, game_link in enumerate(game_links):
        html = urllib.request.urlopen(game_link)
        soup = BeautifulSoup(html, "html.parser")

        # 対戦チーム名
        home = soup.find('title').text.split(" ")[1].split('vs.')[0]
        visitor = soup.find('title').text.split(" ")[1].split('vs.')[1]
    
        # スタメンを抽出
        data = soup.find_all('td', class_='bb-splitsTable__data bb-splitsTable__data--text')
        data = [i.text.split('\n')[1] for i in data][:20]
        home_member = [i.replace(' ', '') for i in data[1:10]]
        visitor_member = [i.replace(' ', '') for i in data[11:20]]

        print(f"home {home}, visitor {visitor}")
        if len(home_member) > 0:
            if not os.path.isdir(data_path+str(game_date)+"/"+str(game_id)):
                os.mkdir(data_path+str(game_date)+"/"+str(game_id))
            visitor_df = make_df(team_dic[visitor], visitor_member, "top")
            home_df = make_df(team_dic[home], home_member, "bottom")

            visitor_df.to_csv(data_path+str(game_date)+"/"+str(game_id) +"/"+str(visitor)+'.txt', index=None, sep=' ', mode='w')
            home_df.to_csv(data_path+str(game_date)+"/"+str(game_id) +"/"+str(home)+'.txt', index=None, sep=' ', mode='w')
        else:
            print("試合中止のためメンバーデータ無し")