"""Migrate legacy stage presentation keys without changing their chosen values."""
import argparse
import configparser
import io
from pathlib import Path


def shared_config(text):
    config=configparser.ConfigParser(interpolation=None,strict=False)
    config.read_string(text.lstrip('\ufeff'))
    def move(old_section,old_key,section,key):
        if not config.has_option(old_section,old_key):return
        if not config.has_section(section):config.add_section(section)
        if not config.has_option(section,key):config[section][key]=config[old_section][old_key]
        config.remove_option(old_section,old_key)
    for old,key in [('render60fps','render60fps'),('stage1_parappa_rail_assist','rail_assist'),
        ('stage1_rail_mode','rail_mode'),('stage1_restore_ceiling_lights','restore_scene_details'),
        ('stage1_hd_geometry_cleanup','geometry_cleanup'),('stage1_texture_replacements','texture_replacements')]:
        move('graphics',old,'presentation',key)
    move('graphics','stage1_texture_replacement_dir','stage_assets','texture_dir')
    for key in ('enabled','aspect_mode'):move('stage2_modern',key,'presentation',key)
    move('stage2_modern','subtitle_file','stage_assets','subtitles_2')
    move('stage1_hd_subtitles','enabled','presentation','hd_subtitles')
    move('stage1_hd_subtitles','file','stage_assets','subtitles_1')
    for old,new in [('stage1_rail_parappa2','rail_feedback'),('stage1_hd_subtitles','hd_subtitles')]:
        if config.has_section(old):
            for key in list(config[old]):
                move(old,key,'presentation' if key in ('scorer_hud','creative_prompt') else new,key)
    for section in ('graphics','stage2_modern','stage1_rail_parappa2','stage1_hd_subtitles'):
        if config.has_section(section) and not config[section]:config.remove_section(section)
    for section in ('presentation','stage_assets'):
        if not config.has_section(section):config.add_section(section)
    for key,value in [('enabled','true'),('aspect_mode','auto')]:
        if not config.has_option('presentation',key):config['presentation'][key]=value
    return config


def serialize(config):
    out=io.StringIO()
    out.write('; Presentation switches apply to every stage.\n'
              '; [presentation] enabled=false: original 4:3, all enhancements off.\n'
              '; aspect_mode=auto / 4:3 / stretch. Shared keys override legacy keys.\n\n')
    config.write(out)
    return out.getvalue()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('config',type=Path)
    parser.add_argument('--output',type=Path,required=True,help='New file; input is never overwritten')
    args=parser.parse_args()
    if args.output.exists():raise FileExistsError(args.output)
    text=serialize(shared_config(args.config.read_text(encoding='utf-8-sig')))
    with args.output.open('x',encoding='utf8') as stream:stream.write(text)
    print(args.output)


if __name__=='__main__':main()
