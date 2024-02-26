def parse_cmap(s):
    rev = False
    if s.endswith('_rev'):
        rev = True
        s = s.removesuffix('_rev')
    if s.startswith('cet:'):
        import colorcet
        s = s.removeprefix('cet:')
        try:
            cmap = getattr(colorcet, s)
        except AttributeError:
            raise ValueError('Unknown colorcet colormap {}'.format(s))
    elif s.startswith('sns:'):
        from matplotlib.colors import rgb2hex
        import seaborn as sns
        s = s.removeprefix('sns:')
        try:
            pal = sns.color_palette(s,as_cmap=True)
            pal_resamp = pal.resampled(256)
            cmap = [rgb2hex(pal_resamp(i)) for i in range(pal_resamp.N)]
        except ValueError:
            raise ValueError('Unknown seaborn colormap {}'.format(s))
    elif s.startswith('cmc:'):
        from matplotlib.colors import rgb2hex
        import cmcrameri.cm as cmc
        s = s.removeprefix('cmc:')
        try:
            pal = getattr(cmc,s)
            pal_resamp = pal.resampled(256)
            cmap = [rgb2hex(pal_resamp(i)) for i in range(pal_resamp.N)]
        except AttributeError:
            raise ValueError('Unknown cmcrameri colormap {}'.format(s))
    else:
        cmap = s.split(',')
    if rev:
        cmap.reverse()
    return cmap
